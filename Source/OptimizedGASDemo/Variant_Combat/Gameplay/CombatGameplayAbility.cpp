// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatGameplayAbility.h"
#include "CombatCharacter.h"
#include "AI/CombatEnemy.h"

UCombatGameplayAbility::UCombatGameplayAbility() {}

void UCombatGameplayAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

  OnAbilityActivated();
}

bool UCombatGameplayAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayTagContainer *SourceTags,
    const FGameplayTagContainer *TargetTags,
    FGameplayTagContainer *OptionalRelevantTags) const {
  return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags,
                                   OptionalRelevantTags);
}

void UCombatGameplayAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {
  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);

  OnAbilityEnded(bWasCancelled);
}

ACombatCharacter *
UCombatGameplayAbility::GetCombatCharacterFromActorInfo() const {
  return Cast<ACombatCharacter>(GetAvatarActorFromActorInfo());
}

ACombatEnemy *UCombatGameplayAbility::GetCombatEnemyFromActorInfo() const {
  return Cast<ACombatEnemy>(GetAvatarActorFromActorInfo());
}
