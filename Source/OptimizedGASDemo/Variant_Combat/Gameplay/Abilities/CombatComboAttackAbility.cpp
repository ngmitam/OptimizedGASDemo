// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatComboAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatBase.h"
#include "CombatCharacter.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagsManager.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Effects/CombatStaminaCostGameplayEffect.h"
#include "Effects/CombatStaminaUsedGameplayEffect.h"
#include "Effects/CombatStaminaUsedResetGameplayEffect.h"

UCombatComboAttackAbility::UCombatComboAttackAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack.Combo")));
  SetAssetTags(AssetTags);

  // Add state tag while active
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Attacking")));

  // Block other attack abilities while this is active
  BlockAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack")));

  // Cancel this ability if death occurs
  CancelAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Death")));

  // Add trigger to activate on combo start event
  FAbilityTriggerData StartTriggerData;
  StartTriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Start"));
  StartTriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(StartTriggerData);

  // Set default stamina cost effect class
  StaminaCostEffectClass = UCombatStaminaCostGameplayEffect::StaticClass();

  // Set default stamina used effect class
  StaminaUsedEffectClass = UCombatStaminaUsedGameplayEffect::StaticClass();
}

void UCombatComboAttackAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  // Get montage and sections from CombatBase
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();

  // Check if stunned, cannot attack
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent(ActorInfo);
  if (ASC->HasMatchingGameplayTag(
          FGameplayTag::RequestGameplayTag(FName("State.Stunned")))) {
    // For enemies, notify StateTree that attack is completed even when stunned
    if (ACombatEnemy *CombatEnemy = Cast<ACombatEnemy>(CombatBase)) {
      CombatEnemy->OnAttackCompleted.ExecuteIfBound();
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  if (CombatBase) {
    ComboAttackMontage = CombatBase->GetComboAttackMontage();
    ComboSectionNames = CombatBase->GetComboSectionNames();
  }

  // Apply stamina cost for first combo hit before committing
  float CurrentStamina =
      ASC->GetNumericAttribute(UStaminaAttributeSet::GetStaminaAttribute());
  if (StaminaCosts.Num() > 0) {
    if (ASC) {
      float StaminaCost = StaminaCosts[0];
      if (CurrentStamina >= StaminaCost) {
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
            StaminaCostEffectClass, 1.0f, ASC->MakeEffectContext());
        if (SpecHandle.IsValid()) {
          // Set the stamina cost using SetByCaller
          SpecHandle.Data.Get()->SetSetByCallerMagnitude(
              FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost")),
              -StaminaCost);
          ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
        }

        // Note: Damage will be set based on last hit's stamina cost when combo
        // ends

        // Set stamina used for damage scaling via GE
        FGameplayEffectSpecHandle UsedSpecHandle = ASC->MakeOutgoingSpec(
            StaminaUsedEffectClass, 1.0f, ASC->MakeEffectContext());
        if (UsedSpecHandle.IsValid()) {
          UsedSpecHandle.Data.Get()->SetSetByCallerMagnitude(
              FGameplayTag::RequestGameplayTag(FName("Data.StaminaUsed")),
              StaminaCost);
          ASC->ApplyGameplayEffectSpecToTarget(*UsedSpecHandle.Data.Get(), ASC);
        }
      } else {
        // Not enough stamina, for enemies set to 0 to refill, end ability
        if (!CombatBase->IsPlayerControlled()) {
          ASC->SetNumericAttributeBase(
              UStaminaAttributeSet::GetStaminaAttribute(), 0.0f);
        }
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
      }
    }
  }

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Set target combo count: max for player, random for AI
  if (CombatBase->IsPlayerControlled()) {
    TargetComboCount = ComboSectionNames.Num();
  } else {
    TargetComboCount = FMath::RandRange(1, ComboSectionNames.Num());
  }

  // Enable root motion on skeletal mesh for attack movement
  if (USkeletalMeshComponent *Mesh = CombatBase->GetMesh()) {
    Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    // Root motion will automatically drive movement during attacks
  }

  // Apply momentum preservation if character has stored velocity
  if (ACombatCharacter *CombatChar = Cast<ACombatCharacter>(CombatBase)) {
    if (!CombatChar->GetStoredVelocity().IsNearlyZero()) {
      CombatBase->GetCharacterMovement()->AddImpulse(
          CombatChar->GetStoredVelocity() *
              CombatChar->GetAttackMomentumMultiplier(),
          true);
    }
  }

  // Notify enemies of attack via ability
  if (ASC) {
    FGameplayEventData EventData;
    EventData.Instigator = TObjectPtr<AActor>(CombatBase);
    EventData.Target = TObjectPtr<AActor>(CombatBase);
    ASC->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Notify.Enemies")),
        &EventData);
  }

  CurrentComboCount = 0;

  // Play montage from first section
  if (ComboAttackMontage && ComboSectionNames.Num() > 0) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Play(ComboAttackMontage, 1.0f);

      // Jump to first section
      AnimInstance->Montage_JumpToSection(ComboSectionNames[0],
                                          ComboAttackMontage);

      // Set end delegate
      FOnMontageEnded MontageEndDelegate;
      MontageEndDelegate.BindUObject(
          this, &UCombatComboAttackAbility::OnMontageEnded);
      AnimInstance->Montage_SetEndDelegate(MontageEndDelegate,
                                           ComboAttackMontage);
    }
  } else {
    // No montage or sections available
  }

  // Listen for combo next event
  if (ASC) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(
            FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Next")))
        .AddUObject(this, &UCombatComboAttackAbility::HandleComboNext);
  }
}

void UCombatComboAttackAbility::OnMontageEnded(UAnimMontage *Montage,
                                               bool bInterrupted) {
  // Reset stamina used for next attack via GE
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());
  if (ASC) {
    FGameplayEffectSpecHandle ResetSpecHandle = ASC->MakeOutgoingSpec(
        UCombatStaminaUsedResetGameplayEffect::StaticClass(), 1.0f,
        ASC->MakeEffectContext());
    if (ResetSpecHandle.IsValid()) {
      ASC->ApplyGameplayEffectSpecToTarget(*ResetSpecHandle.Data.Get(), ASC);
    }
  }

  // Call the character's attack montage ended function
  if (AActor *Avatar = GetCurrentActorInfo()->AvatarActor.Get()) {
    if (ACombatBase *CombatBase = Cast<ACombatBase>(Avatar)) {
      CombatBase->AttackMontageEnded(Montage, bInterrupted);
    }
  }

  EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
             GetCurrentActivationInfo(), false, bInterrupted);
}

void UCombatComboAttackAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {
  // Stop montage only if cancelled
  if (bWasCancelled && ComboAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Stop(0.0f, ComboAttackMontage);
    }
  }

  // Remove montage end delegate only if cancelled
  if (bWasCancelled && ComboAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      FOnMontageEnded EmptyDelegate;
      AnimInstance->Montage_SetEndDelegate(EmptyDelegate, ComboAttackMontage);
    }
  }

  // Remove event listeners
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent(ActorInfo)) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(
            FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Next")))
        .RemoveAll(this);
  }

  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);
}

void UCombatComboAttackAbility::HandleComboNext(
    const FGameplayEventData *EventData) {
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());

  bool bShouldContinueCombo = true;

  if (CombatBase) {
    if (CombatBase->IsPlayerControlled()) {
      // For player, check if input is not stale
      ACombatCharacter *CombatChar = Cast<ACombatCharacter>(CombatBase);
      if (CombatChar) {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - CombatChar->GetCachedComboAttackInputTime() >
            CombatBase->GetComboInputCacheTimeTolerance()) {
          // Input is stale, end combo
          bShouldContinueCombo = false;
        } else {
          // Consume the cached input
          CombatChar->SetCachedComboAttackInputTime(0.0f);
          bShouldContinueCombo = true;
        }
      }
    } else {
      // For AI, always continue until target combo count
      bShouldContinueCombo = true;
    }
  }

  if (bShouldContinueCombo) {
    CurrentComboCount++;

    // Check if we have more combos to play
    if (CurrentComboCount < TargetComboCount &&
        CurrentComboCount < ComboSectionNames.Num()) {
      // Apply stamina cost for next combo hit
      if (StaminaCosts.Num() > 0) {
        int32 CostIndex = FMath::Min(CurrentComboCount, StaminaCosts.Num() - 1);
        float StaminaCost = StaminaCosts[CostIndex];
        if (ASC) {
          // Check if we have enough stamina
          float CurrentStamina = ASC->GetNumericAttribute(
              UStaminaAttributeSet::GetStaminaAttribute());
          if (CurrentStamina >= StaminaCost) {
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
                StaminaCostEffectClass, 1.0f, ASC->MakeEffectContext());
            if (SpecHandle.IsValid()) {
              // Set the stamina cost using SetByCaller
              SpecHandle.Data.Get()->SetSetByCallerMagnitude(
                  FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost")),
                  -StaminaCost);
              ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
            }

            // Note: Damage will be set based on last hit's stamina cost when
            // combo ends

            // Set stamina used for damage scaling (each hit scales
            // individually) via GE
            FGameplayEffectSpecHandle UsedSpecHandle = ASC->MakeOutgoingSpec(
                StaminaUsedEffectClass, 1.0f, ASC->MakeEffectContext());
            if (UsedSpecHandle.IsValid()) {
              UsedSpecHandle.Data.Get()->SetSetByCallerMagnitude(
                  FGameplayTag::RequestGameplayTag(FName("Data.StaminaUsed")),
                  StaminaCost);
              ASC->ApplyGameplayEffectSpecToTarget(*UsedSpecHandle.Data.Get(),
                                                   ASC);
            }

            // Jump to next section
            if (UAnimInstance *AnimInstance =
                    GetCurrentActorInfo()->GetAnimInstance()) {
              AnimInstance->Montage_JumpToSection(
                  ComboSectionNames[CurrentComboCount], ComboAttackMontage);
            }
          } else {
            // Not enough stamina, for enemies set to 0 to refill, end combo
            if (!CombatBase->IsPlayerControlled()) {
              ASC->SetNumericAttributeBase(
                  UStaminaAttributeSet::GetStaminaAttribute(), 0.0f);
            }
            EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
                       GetCurrentActivationInfo(), false, false);
            return;
          }
        }
      } else {
        // Jump to next section (no stamina cost)
        if (UAnimInstance *AnimInstance =
                GetCurrentActorInfo()->GetAnimInstance()) {
          AnimInstance->Montage_JumpToSection(
              ComboSectionNames[CurrentComboCount], ComboAttackMontage);
        }
      }
    } else {
      // No more combos, end ability
      EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
                 GetCurrentActivationInfo(), false, false);
    }
  } else {
    // End combo due to no input
    EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
               GetCurrentActivationInfo(), false, false);
  }
}
