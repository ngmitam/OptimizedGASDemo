// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatComboAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatBase.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagsManager.h"

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
}

void UCombatComboAttackAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Get montage and sections from CombatBase
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();

  if (CombatBase) {
    ComboAttackMontage = CombatBase->GetComboAttackMontage();
    ComboSectionNames = CombatBase->GetComboSectionNames();
    // Set target combo count: max for player, random for AI
    if (CombatBase->IsPlayerControlled()) {
      TargetComboCount = ComboSectionNames.Num();
    } else {
      TargetComboCount = FMath::RandRange(1, ComboSectionNames.Num());
    }

    // Set attacking flag
    CombatBase->SetIsAttacking(true);
    // Notify enemies of attack via ability
    if (UAbilitySystemComponent *ASC =
            GetAbilitySystemComponentFromActorInfo()) {
      FGameplayEventData EventData;
      EventData.Instigator = TObjectPtr<AActor>(CombatBase);
      EventData.Target = TObjectPtr<AActor>(CombatBase);
      ASC->HandleGameplayEvent(
          FGameplayTag::RequestGameplayTag(FName("Event.Notify.Enemies")),
          &EventData);
    }
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
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo()) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(
            FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Next")))
        .AddUObject(this, &UCombatComboAttackAbility::HandleComboNext);
  }
}

void UCombatComboAttackAbility::OnMontageEnded(UAnimMontage *Montage,
                                               bool bInterrupted) {
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
  // Reset attacking flag
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();
  if (CombatBase) {
    CombatBase->SetIsAttacking(false);
  }

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
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo()) {
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

  bool bShouldContinueCombo = true;

  if (CombatBase) {
    if (CombatBase->IsPlayerControlled()) {
      // For player, check if input is not stale
      float CurrentTime = GetWorld()->GetTimeSeconds();
      if (CurrentTime - CombatBase->GetCachedComboAttackInputTime() >
          CombatBase->GetComboInputCacheTimeTolerance()) {
        // Input is stale, end combo
        bShouldContinueCombo = false;
      } else {
        // Consume the input
        CombatBase->SetCachedComboAttackInputTime(0.0f);
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
      // Jump to next section
      if (UAnimInstance *AnimInstance =
              GetCurrentActorInfo()->GetAnimInstance()) {
        AnimInstance->Montage_JumpToSection(
            ComboSectionNames[CurrentComboCount], ComboAttackMontage);
      }
    } else {
      // No more combos, end ability
      EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
                 GetCurrentActivationInfo(), false, false);
    }
  } else {
    // End combo due to stale input
    EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
               GetCurrentActivationInfo(), false, false);
  }
}
