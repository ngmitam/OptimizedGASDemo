// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatNotifyEnemiesAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatCharacter.h"
#include "AI/CombatEnemy.h"
#include "GameplayTagsManager.h"

UCombatNotifyEnemiesAbility::UCombatNotifyEnemiesAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Notify.Enemies")));
  SetAssetTags(AssetTags);

  // Add trigger to activate on notify enemies event
  FAbilityTriggerData TriggerData;
  TriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Notify.Enemies"));
  TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(TriggerData);
}

void UCombatNotifyEnemiesAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Perform notify enemies trace
  if (!CurrentActorInfo->AvatarActor.IsValid()) {
    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    return;
  }

  AActor *AvatarActor = CurrentActorInfo->AvatarActor.Get();

  // Get trace parameters from character
  float DangerTraceDistance = 300.0f;
  float DangerTraceRadius = 75.0f;

  if (ACombatCharacter *CombatChar = Cast<ACombatCharacter>(AvatarActor)) {
    DangerTraceDistance = CombatChar->GetDangerTraceDistance();
    DangerTraceRadius = CombatChar->GetDangerTraceRadius();
  } else if (ACombatEnemy *CombatEnemy = Cast<ACombatEnemy>(AvatarActor)) {
    // Assuming CombatEnemy has similar properties, or use defaults
    // For now, use defaults
  }

  // Sweep for objects in front of the character to be hit by the attack
  TArray<FHitResult> OutHits;

  // Start at the actor location, sweep forward
  const FVector TraceStart = AvatarActor->GetActorLocation();
  const FVector TraceEnd =
      TraceStart + (AvatarActor->GetActorForwardVector() * DangerTraceDistance);

  // Check for pawn object types only
  FCollisionObjectQueryParams ObjectParams;
  ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

  // Use a sphere shape for the sweep
  FCollisionShape CollisionShape;
  CollisionShape.SetSphere(DangerTraceRadius);

  // Ignore self
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(AvatarActor);

  if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd,
                                         FQuat::Identity, ObjectParams,
                                         CollisionShape, QueryParams)) {
    // Iterate over each object hit
    for (const FHitResult &CurrentHit : OutHits) {
      // Check if we've hit a damageable actor
      ICombatDamageable *Damageable =
          Cast<ICombatDamageable>(CurrentHit.GetActor());

      if (Damageable) {
        // Notify the enemy
        Damageable->NotifyDanger(TraceStart, AvatarActor);
      }
    }
  }

  // End ability immediately after performing trace
  EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
