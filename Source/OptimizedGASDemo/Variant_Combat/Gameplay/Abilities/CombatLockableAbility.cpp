// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockableAbility.h"
#include "AbilitySystemComponent.h"

UCombatLockableAbility::UCombatLockableAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

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

  // Try to activate the ability immediately upon granting (server-only for
  // multiplayer safety)
  if (ActorInfo && GetAbilitySystemComponent(ActorInfo) &&
      ActorInfo->AvatarActor.Get() &&
      ActorInfo->AvatarActor.Get()->HasAuthority()) {
    GetAbilitySystemComponent(ActorInfo)->TryActivateAbility(Spec.Handle);
  }
}
