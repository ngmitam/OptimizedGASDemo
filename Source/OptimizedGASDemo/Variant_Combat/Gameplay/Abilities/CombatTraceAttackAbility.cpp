// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatTraceAttackAbility.h"
#include "CombatBase.h"
#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Effects/CombatDamageGameplayEffect.h"
#include "Attributes/DamageAttributeSet.h"
#include "Gameplay/Attributes/StaminaAttributeSet.h"
#include "Data/CombatAttackEventData.h"

UCombatTraceAttackAbility::UCombatTraceAttackAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Trace.Attack")));
  SetAssetTags(AssetTags);

  // Add state tag while active
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Attacking")));

  // Cancel this ability if death occurs
  CancelAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Death")));

  // Add trigger to activate on attack start event
  FAbilityTriggerData TriggerData;
  TriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Trace.Attack"));
  TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(TriggerData);
}

void UCombatTraceAttackAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Store trigger event data for use in PerformAttackTrace
  CachedTriggerEventData = TriggerEventData;

  // Perform attack trace immediately
  PerformAttackTrace();

  // End ability immediately after performing trace
  EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UCombatTraceAttackAbility::PerformAttackTrace() {
  if (!CurrentActorInfo->AvatarActor.IsValid()) {
    return;
  }

  AActor *AvatarActor = CurrentActorInfo->AvatarActor.Get();

  // Get damage from GAS attribute
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent(CurrentActorInfo);
  float BaseDamage = DamageAmount;
  if (ASC) {
    BaseDamage = DamageAmount + ASC->GetNumericAttribute(
                                    UDamageAttributeSet::GetDamageAttribute());
  }

  // Get knockback and launch from GAS attributes
  float KnockbackValue = KnockbackImpulse;
  float LaunchValue = LaunchImpulse;
  if (ASC) {
    KnockbackValue = ASC->GetNumericAttribute(
        UDamageAttributeSet::GetKnockbackImpulseAttribute());
    LaunchValue = ASC->GetNumericAttribute(
        UDamageAttributeSet::GetLaunchImpulseAttribute());
  }

  // Get trace parameters from ability properties
  float TraceDistanceValue = TraceDistance;
  float TraceRadiusValue = TraceRadius;

  // Sweep for objects in front of the character to be hit by the attack
  TArray<FHitResult> OutHits;

  // Start at the actor location, sweep forward (use socket location if
  // available)
  FVector TraceStart = AvatarActor->GetActorLocation();

  // Get damage source bone from event data if available
  FName DamageSourceBone = FName("hand_r"); // default
  if (CachedTriggerEventData && CachedTriggerEventData->OptionalObject) {
    if (const UCombatAttackEventData *AttackData = Cast<UCombatAttackEventData>(
            CachedTriggerEventData->OptionalObject)) {
      DamageSourceBone = AttackData->DamageSourceBone;
    }
  }

  // Get stamina used from GAS attribute for damage scaling
  float StaminaUsed = 0.0f;
  if (ASC) {
    StaminaUsed = ASC->GetNumericAttribute(
        UStaminaAttributeSet::GetStaminaUsedAttribute());
  }

  // Scale damage based on stamina used (higher stamina cost = higher damage)
  // Damage : base damage + (stamina used * StaminaDamageMultiplier)
  float Damage = BaseDamage + (StaminaUsed * StaminaDamageMultiplier);

  if (ACharacter *Character = Cast<ACharacter>(AvatarActor)) {
    if (Character->GetMesh()) {
      // Try to get socket location from the damage source bone
      FVector SocketLocation =
          Character->GetMesh()->GetSocketLocation(DamageSourceBone);
      if (!SocketLocation.IsZero()) {
        TraceStart = SocketLocation;
      }
    }
  }
  const FVector TraceEnd =
      TraceStart + (AvatarActor->GetActorForwardVector() * TraceDistanceValue);

  // Check for pawn and world dynamic collision object types
  FCollisionObjectQueryParams ObjectParams;
  ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
  ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

  // Use a sphere shape for the sweep
  FCollisionShape CollisionShape;
  CollisionShape.SetSphere(TraceRadiusValue);

  // Ignore self
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(AvatarActor);

  bool bHit = GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd,
                                                 FQuat::Identity, ObjectParams,
                                                 CollisionShape, QueryParams);

  // Draw debug trace visualization
  DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 2.0f, 0,
                2.0f);
  DrawDebugSphere(GetWorld(), TraceStart, TraceRadiusValue, 12, FColor::Blue,
                  false, 2.0f);
  DrawDebugSphere(GetWorld(), TraceEnd, TraceRadiusValue, 12, FColor::Blue,
                  false, 2.0f);

  // Process all unique hit actors (each actor only once)
  TSet<AActor *> ProcessedActors;
  for (const FHitResult &HitResult : OutHits) {
    AActor *HitActor = HitResult.GetActor();
    if (HitActor && !ProcessedActors.Contains(HitActor)) {
      ProcessedActors.Add(HitActor);

      // Draw Debug Hit
      DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 12, FColor::Red,
                      false, 2.0f);

      ICombatDamageable *Damageable = Cast<ICombatDamageable>(HitActor);
      if (Damageable) {
        // Notify danger to the hit actor
        Damageable->NotifyDanger(HitResult.ImpactPoint, AvatarActor);

        // knock upwards and away from the impact normal
        const FVector Impulse = (HitResult.ImpactNormal * -KnockbackValue) +
                                (FVector::UpVector * LaunchValue);
        Damageable->ApplyDamage(Damage, AvatarActor, HitResult.ImpactPoint,
                                Impulse);
      }

      // Call DealtDamage for visual effects on attacker
      ACombatCharacter *CombatCharacter = GetCombatCharacterFromActorInfo();
      if (CombatCharacter) {
        CombatCharacter->DealtDamage(Damage, HitResult.ImpactPoint);
      }
    }
  }
}

ACombatCharacter *
UCombatTraceAttackAbility::GetCombatCharacterFromActorInfo() const {
  return Cast<ACombatCharacter>(GetAvatarActorFromActorInfo());
}
