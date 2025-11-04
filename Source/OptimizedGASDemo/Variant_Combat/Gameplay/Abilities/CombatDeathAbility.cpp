// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatDeathAbility.h"
#include "CombatCharacter.h"
#include "AI/CombatEnemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

UCombatDeathAbility::UCombatDeathAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Death")));
  SetAssetTags(AssetTags);

  // Block all abilities while dead
  BlockAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability")));

  // Add trigger to activate on death event
  FAbilityTriggerData TriggerData;
  TriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Death"));
  TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(TriggerData);
}

void UCombatDeathAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Disable character movement and collision
  if (ActorInfo->AvatarActor.IsValid()) {
    ACharacter *Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (Character) {
      Character->GetCharacterMovement()->DisableMovement();
      Character->GetCapsuleComponent()->SetCollisionEnabled(
          ECollisionEnabled::NoCollision);
    }
  }

  // Start death timer
  GetWorld()->GetTimerManager().SetTimer(
      DeathTimer, this, &UCombatDeathAbility::OnDeathTimerExpired,
      DeathDuration, false);
}

void UCombatDeathAbility::OnDeathTimerExpired() {
  if (CurrentActorInfo->AvatarActor.IsValid()) {
    ACombatCharacter *CombatCharacter = GetCombatCharacterFromActorInfo();
    ACombatEnemy *CombatEnemy = GetCombatEnemyFromActorInfo();

    if (CombatCharacter && bShouldRespawn) {
      // Respawn character
      CombatCharacter->RespawnCharacter();
    } else if (CombatEnemy) {
      // Remove enemy
      CombatEnemy->RemoveFromLevel();
    }
  }

  EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false,
             false);
}
