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
  if (CombatChar) {
    if (CombatChar->HasLockedTarget()) {
      CombatChar->UnlockTarget();
      // Re-enable pawn control rotation
      if (CombatChar->GetCameraBoom()) {
        CombatChar->GetCameraBoom()->bUsePawnControlRotation = true;
      }
      // Remove locked state tag
      if (CombatChar->GetAbilitySystemComponent()) {
        CombatChar->GetAbilitySystemComponent()->RemoveLooseGameplayTag(
            FGameplayTag::RequestGameplayTag(FName("State.Locked")));
      }
    } else {
      CombatChar->LockOntoTarget();
      // Disable pawn control rotation for camera lock
      if (CombatChar->GetCameraBoom()) {
        CombatChar->GetCameraBoom()->bUsePawnControlRotation = false;
      }
      // Add locked state tag
      if (CombatChar->GetAbilitySystemComponent()) {
        CombatChar->GetAbilitySystemComponent()->AddLooseGameplayTag(
            FGameplayTag::RequestGameplayTag(FName("State.Locked")));
      }
    }
  }

  EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
