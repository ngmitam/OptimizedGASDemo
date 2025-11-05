// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockableAbility.h"
#include "AbilitySystemComponent.h"

UCombatLockableAbility::UCombatLockableAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set up ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Passive.lockable")));
  AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Lockable")));
  SetAssetTags(AssetTags);
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Lockable")));
}

void UCombatLockableAbility::OnGiveAbility(
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilitySpec &Spec) {
  Super::OnGiveAbility(ActorInfo, Spec);

  // Try to activate the ability immediately upon granting
  if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid()) {
    ActorInfo->AbilitySystemComponent.Get()->TryActivateAbility(Spec.Handle);
  }
}
