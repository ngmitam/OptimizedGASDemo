// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatChargedAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatBase.h"
#include "CombatCharacter.h"
#include "AI/CombatEnemy.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagsManager.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Effects/CombatStaminaCostGameplayEffect.h"
#include "Effects/CombatStaminaUsedGameplayEffect.h"
#include "Effects/CombatStaminaUsedResetGameplayEffect.h"

UCombatChargedAttackAbility::UCombatChargedAttackAbility() {
  // Set up ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack.Charged")));
  SetAssetTags(AssetTags);

  // Add state tag while active
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.ChargingAttack")));

  // Block other attack abilities while charging
  BlockAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack")));

  // Cancel this ability if death occurs
  CancelAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Death")));

  // Add trigger to activate on charged start event
  FAbilityTriggerData StartTriggerData;
  StartTriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Start"));
  StartTriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(StartTriggerData);

  // Set default stamina cost effect class
  StaminaCostEffectClass = UCombatStaminaCostGameplayEffect::StaticClass();

  // Set default stamina used effect class
  StaminaUsedEffectClass = UCombatStaminaUsedGameplayEffect::StaticClass();
}

void UCombatChargedAttackAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  // Check if stunned, cannot attack
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent(ActorInfo);
  if (ASC->HasMatchingGameplayTag(
          FGameplayTag::RequestGameplayTag(FName("State.Stunned")))) {
    // For enemies, notify StateTree that attack is completed even when stunned
    ACombatBase *CombatBase = GetCombatBaseFromActorInfo();
    if (ACombatEnemy *CombatEnemy = Cast<ACombatEnemy>(CombatBase)) {
      CombatEnemy->OnAttackCompleted.ExecuteIfBound();
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Determine if player or AI and get montage
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();

  // Check if we have enough stamina for at least one loop
  float CurrentStamina =
      ASC->GetNumericAttribute(UStaminaAttributeSet::GetStaminaAttribute());
  if (ASC && StaminaCosts.Num() > 0) {
    float StaminaCost = StaminaCosts[0];
    if (CurrentStamina < StaminaCost) {
      // Not enough stamina to start charging, for enemies set to 0 to refill,
      // end ability
      if (CombatBase && !CombatBase->IsPlayerControlled()) {
        ASC->SetNumericAttributeBase(
            UStaminaAttributeSet::GetStaminaAttribute(), 0.0f);
      }
      EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
      return;
    }

    // Apply stamina cost for the first loop immediately
    // Set stamina used attribute for execution calculation
    ASC->SetNumericAttributeBase(
        UStaminaAttributeSet::GetStaminaUsedAttribute(), StaminaCost);

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        StaminaCostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
    if (SpecHandle.IsValid()) {
      ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
    }

    // Set stamina used for damage scaling
    TotalStaminaUsed = StaminaCost;
  }

  if (CombatBase) {
    if (Cast<ACombatCharacter>(CombatBase)) {
      bIsPlayerControlled = true;
    } else if (ACombatEnemy *CombatEnemy = Cast<ACombatEnemy>(CombatBase)) {
      bIsPlayerControlled = false;
      // For AI, set random target loops
      TargetChargeLoops = FMath::RandRange(CombatEnemy->GetMinChargeLoops(),
                                           CombatEnemy->GetMaxChargeLoops());
    }

    ChargedAttackMontage = CombatBase->GetChargedAttackMontage();
    ChargeLoopSection = CombatBase->GetChargeLoopSection();
    ChargeAttackSection = CombatBase->GetChargeAttackSection();

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

    // Add camera shake for feedback (only for player)
    if (ACombatCharacter *CombatChar = Cast<ACombatCharacter>(CombatBase)) {
      if (CombatChar->GetAttackCameraShake() &&
          CombatBase->IsPlayerControlled()) {
        if (APlayerController *PC =
                Cast<APlayerController>(CombatBase->GetController())) {
          if (PC->PlayerCameraManager) {
            PC->PlayerCameraManager->StartCameraShake(
                CombatChar->GetAttackCameraShake());
          }
        }
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
  }

  CurrentChargeLoop = 1;
  bAutoReleased = false;

  // Play montage
  if (ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f);

      // Set end delegate
      FOnMontageEnded MontageEndDelegate;
      MontageEndDelegate.BindUObject(
          this, &UCombatChargedAttackAbility::OnMontageEnded);
      AnimInstance->Montage_SetEndDelegate(MontageEndDelegate,
                                           ChargedAttackMontage);
    }
  } else {
    // No montage available
  }

  // Listen for events
  if (ASC) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Loop")))
        .AddUObject(this,
                    &UCombatChargedAttackAbility::HandleChargedAttackLoop);
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Release")))
        .AddUObject(this,
                    &UCombatChargedAttackAbility::HandleChargedAttackRelease);
  }
}

void UCombatChargedAttackAbility::OnMontageEnded(UAnimMontage *Montage,
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

void UCombatChargedAttackAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {
  // Stop montage only if cancelled
  if (bWasCancelled && ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Stop(0.0f, ChargedAttackMontage);
    }
  }

  // Remove montage end delegate only if cancelled
  if (bWasCancelled && ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      FOnMontageEnded EmptyDelegate;
      AnimInstance->Montage_SetEndDelegate(EmptyDelegate, ChargedAttackMontage);
    }
  }

  // Remove event listeners
  if (UAbilitySystemComponent *ASC =
          GetAbilitySystemComponent(CurrentActorInfo)) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Loop")))
        .RemoveAll(this);
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Release")))
        .RemoveAll(this);
  }

  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);
}

void UCombatChargedAttackAbility::HandleChargedAttackLoop(
    const FGameplayEventData *EventData) {
  // Apply stamina cost for each loop like a combo hit
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent(CurrentActorInfo);
  if (ASC && StaminaCosts.Num() > 0) {
    float StaminaCost = StaminaCosts[0]; // Use first cost for all loops

    // Check if we have enough stamina
    float CurrentStamina =
        ASC->GetNumericAttribute(UStaminaAttributeSet::GetStaminaAttribute());
    if (CurrentStamina < StaminaCost) {
      // Not enough stamina, for enemies set to 0 to refill, automatically
      // release the attack
      if (!bIsPlayerControlled) {
        ASC->SetNumericAttributeBase(
            UStaminaAttributeSet::GetStaminaAttribute(), 0.0f);
      }
      HandleChargedAttackRelease(EventData);
      bAutoReleased = true;
      return;
    }

    // Apply stamina cost effect
    // Set stamina used attribute for execution calculation
    ASC->SetNumericAttributeBase(
        UStaminaAttributeSet::GetStaminaUsedAttribute(), StaminaCost);

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        StaminaCostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
    if (SpecHandle.IsValid()) {
      ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
    }

    // Update total stamina used
    TotalStaminaUsed += StaminaCost;
  }

  if (bIsPlayerControlled) {
    // For player, always loop if still holding
    if (UAnimInstance *AnimInstance =
            GetCurrentActorInfo()->GetAnimInstance()) {
      AnimInstance->Montage_JumpToSection(ChargeLoopSection,
                                          ChargedAttackMontage);
    }
  } else {
    // For AI, check if we need to loop more
    CurrentChargeLoop++;
    if (CurrentChargeLoop < TargetChargeLoops) {
      if (UAnimInstance *AnimInstance =
              GetCurrentActorInfo()->GetAnimInstance()) {
        AnimInstance->Montage_JumpToSection(ChargeLoopSection,
                                            ChargedAttackMontage);
      }
    } else {
      // Go to attack
      if (UAnimInstance *AnimInstance =
              GetCurrentActorInfo()->GetAnimInstance()) {
        AnimInstance->Montage_JumpToSection(ChargeAttackSection,
                                            ChargedAttackMontage);
      }
    }
  }
}

void UCombatChargedAttackAbility::HandleChargedAttackRelease(
    const FGameplayEventData *EventData) {
  // If already auto-released due to insufficient stamina, do nothing
  if (bAutoReleased) {
    return;
  }

  // Set stamina used for damage scaling via GE
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());
  if (ASC) {
    FGameplayEffectSpecHandle UsedSpecHandle = ASC->MakeOutgoingSpec(
        StaminaUsedEffectClass, 1.0f, ASC->MakeEffectContext());
    if (UsedSpecHandle.IsValid()) {
      UsedSpecHandle.Data.Get()->SetSetByCallerMagnitude(
          FGameplayTag::RequestGameplayTag(FName("Data.StaminaUsed")),
          TotalStaminaUsed);
      ASC->ApplyGameplayEffectSpecToTarget(*UsedSpecHandle.Data.Get(), ASC);
    }
  }

  // Release attack
  if (UAnimInstance *AnimInstance = GetCurrentActorInfo()->GetAnimInstance()) {
    AnimInstance->Montage_JumpToSection(ChargeAttackSection,
                                        ChargedAttackMontage);
  }
}
