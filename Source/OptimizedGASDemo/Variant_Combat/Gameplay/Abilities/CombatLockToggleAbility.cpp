// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockToggleAbility.h"
#include "CombatCharacter.h"
#include "LockSystem/CombatLockSystemComponent.h"

UCombatLockToggleAbility::UCombatLockToggleAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Lock.Toggle")));
  SetAssetTags(AssetTags);

  // Add trigger to activate on lock toggle event
  FAbilityTriggerData TriggerData;
  TriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Lock.Toggle"));
  TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(TriggerData);
}

void UCombatLockToggleAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  ACombatCharacter *CombatChar =
      Cast<ACombatCharacter>(ActorInfo->AvatarActor.Get());
  if (CombatChar && CombatChar->GetLockSystemComponent()) {
    // Server reconciliation: let server validate and set lock state
    if (ActorInfo->AvatarActor->HasAuthority()) {
      // Server: perform the actual lock/unlock logic
      Server_ToggleLockState(CombatChar);
    } else {
      // Client: predict the toggle and request server confirmation
      Client_PredictToggleLock(CombatChar);
    }
  }

  EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UCombatLockToggleAbility::Server_ToggleLockState(
    ACombatCharacter *CombatChar) {
  if (!CombatChar || !CombatChar->GetLockSystemComponent()) {
    return;
  }

  UCombatLockSystemComponent *LockSystem = CombatChar->GetLockSystemComponent();

  if (LockSystem->HasLockedTarget()) {
    // Validate that target is still valid before unlocking
    if (LockSystem->IsTargetStillValid()) {
      LockSystem->UnlockTarget();
    } else {
      // Target became invalid, force unlock
      LockSystem->UnlockTarget();
    }
  } else {
    // Try to lock onto a new target
    LockSystem->LockOntoTarget();
  }
}

void UCombatLockToggleAbility::Client_PredictToggleLock(
    ACombatCharacter *CombatChar) {
  if (!CombatChar || !CombatChar->GetLockSystemComponent()) {
    return;
  }

  UCombatLockSystemComponent *LockSystem = CombatChar->GetLockSystemComponent();

  // Client prediction: immediately toggle for responsive feel
  if (LockSystem->HasLockedTarget()) {
    LockSystem->UnlockTarget();
  } else {
    LockSystem->LockOntoTarget();
  }

  // Server will reconcile via replication
}
