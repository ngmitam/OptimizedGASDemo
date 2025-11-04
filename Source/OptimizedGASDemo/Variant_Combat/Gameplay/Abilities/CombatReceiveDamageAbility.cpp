// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatReceiveDamageAbility.h"
#include "CombatCharacter.h"
#include "AI/CombatEnemy.h"
#include "Interfaces/CombatDamageable.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/Attributes/CombatAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Effects/CombatDamageGameplayEffect.h"

UCombatReceiveDamageAbility::UCombatReceiveDamageAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

  // Set ability tags for event activation
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Event.Damage.Received")));

  // Ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Damage.Receive")));
  SetAssetTags(AssetTags);

  // Block attack abilities while receiving damage
  BlockAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Attack")));

  // Add trigger to activate on damage received event
  FAbilityTriggerData TriggerData;
  TriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Damage.Received"));
  TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(TriggerData);

  // Set default damage gameplay effect
  DamageGameplayEffect = UCombatDamageGameplayEffect::StaticClass();
}

void UCombatReceiveDamageAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Get damage data from trigger event
  float Damage = TriggerEventData ? TriggerEventData->EventMagnitude : 0.0f;
  FVector ImpactPoint = FVector::ZeroVector;
  FVector DamageDirection = FVector::ZeroVector;
  if (TriggerEventData) {
    const UDamageEventData *DamageData =
        Cast<UDamageEventData>(TriggerEventData->OptionalObject);
    if (DamageData) {
      ImpactPoint = DamageData->Location;
      DamageDirection = DamageData->Impulse;
    } else {
      // Fallback to calculation
      ImpactPoint = TriggerEventData->Target
                        ? TriggerEventData->Target->GetActorLocation()
                        : FVector::ZeroVector;
      DamageDirection =
          TriggerEventData->Instigator
              ? (TriggerEventData->Instigator->GetActorLocation() - ImpactPoint)

              : FVector::ZeroVector;
    }
  }

  // Draw debug sphere at impact point
  DrawDebugSphere(GetWorld(), ImpactPoint, 15.0f, 12, FColor::Yellow, false,
                  3.0f);

  // Apply damage via Gameplay Effect
  if (ActorInfo->AbilitySystemComponent.IsValid() && DamageGameplayEffect) {
    FGameplayEffectSpecHandle DamageSpecHandle =
        ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
            DamageGameplayEffect, 1.0f,
            ActorInfo->AbilitySystemComponent->MakeEffectContext());
    if (DamageSpecHandle.IsValid()) {
      DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(
          FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -Damage);
      ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
          *DamageSpecHandle.Data.Get(),
          ActorInfo->AbilitySystemComponent.Get());
    }
  }

  ApplyDamageEffects(Damage, ImpactPoint, DamageDirection);

  // Check if character should die
  const UCombatAttributeSet *ConstAttributeSet = Cast<UCombatAttributeSet>(
      ActorInfo->AbilitySystemComponent->GetAttributeSet(
          UCombatAttributeSet::StaticClass()));
  UCombatAttributeSet *AttributeSet =
      const_cast<UCombatAttributeSet *>(ConstAttributeSet);
  if (AttributeSet && AttributeSet->GetHealth() <= 0.0f) {
    // Call HandleDeath instead of playing montage
    if (ActorInfo->AvatarActor.IsValid()) {
      ICombatDamageable *Damageable =
          Cast<ICombatDamageable>(ActorInfo->AvatarActor.Get());
      if (Damageable) {
        Damageable->HandleDeath();
      }
    }

    // Play death camera shake
    if (DeathCameraShake) {
      UGameplayStatics::PlayWorldCameraShake(this, DeathCameraShake,
                                             ImpactPoint, 0.0f, 1000.0f);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
  } else {
    // Play damage camera shake
    if (DamageCameraShake) {
      UGameplayStatics::PlayWorldCameraShake(this, DamageCameraShake,
                                             ImpactPoint, 0.0f, 1000.0f);
    }

    // Call ReceivedDamage for visual effects on character
    if (ActorInfo->AvatarActor.IsValid()) {
      // Check if actor is CombatCharacter or CombatEnemy
      ACombatCharacter *CombatChar = GetCombatCharacterFromActorInfo();
      ACombatEnemy *CombatEnemy = GetCombatEnemyFromActorInfo();
      if (CombatChar) {
        CombatChar->ReceivedDamage(Damage, ImpactPoint, DamageDirection);
      } else if (CombatEnemy) {

        // stop the attack montages to interrupt the attack
        if (UAnimInstance *AnimInstance =
                CombatEnemy->GetMesh()->GetAnimInstance()) {
          AnimInstance->Montage_Stop(0.1f,
                                     CombatEnemy->GetComboAttackMontage());
          AnimInstance->Montage_Stop(0.1f,
                                     CombatEnemy->GetChargedAttackMontage());
        }
        CombatEnemy->ReceivedDamage(Damage, ImpactPoint, DamageDirection);
      }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
  }
}

void UCombatReceiveDamageAbility::ApplyDamageEffects(
    float Damage, const FVector &ImpactPoint, const FVector &DamageDirection) {
  if (!CurrentActorInfo->AvatarActor.IsValid()) {
    return;
  }

  ACharacter *Character = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
  if (!Character) {
    return;
  }

  // Apply knockbackbut keep the pelvis vertical
  if (ACombatCharacter *CombatChar = Cast<ACombatCharacter>(Character)) {
    CombatChar->GetMesh()->SetPhysicsBlendWeight(0.5f);
    CombatChar->GetMesh()->SetBodySimulatePhysics(
        CombatChar->GetPelvisBoneName(), false);
  } else if (ACombatEnemy *CombatEnemy = Cast<ACombatEnemy>(Character)) {
    CombatEnemy->GetMesh()->SetPhysicsBlendWeight(0.5f);
    CombatEnemy->GetMesh()->SetBodySimulatePhysics(
        CombatEnemy->GetPelvisBoneName(), false);
  }

  // apply knockback impulse
  Character->GetCharacterMovement()->AddImpulse(DamageDirection, true);
  // is the character ragdolling?
  if (Character->GetMesh()->IsSimulatingPhysics()) {
    // apply an impulse to the ragdoll
    Character->GetMesh()->AddImpulseAtLocation(
        DamageDirection * Character->GetMesh()->GetMass(), ImpactPoint);
  }
}
