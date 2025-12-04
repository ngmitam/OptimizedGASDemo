// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatReceiveDamageAbility.h"
#include "CombatBase.h"
#include "CombatCharacter.h"
#include "CombatPlayerState.h"
#include "AI/CombatEnemy.h"
#include "Interfaces/CombatDamageable.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Effects/CombatDamageGameplayEffect.h"

UCombatReceiveDamageAbility::UCombatReceiveDamageAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

  // Ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Damage.Receive")));
  SetAssetTags(AssetTags);

  // Block attack abilities while receiving damage
  BlockAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack")));

  // Add state tag while active
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Damaged")));

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
    const UCombatDamageEventData *DamageData =
        Cast<UCombatDamageEventData>(TriggerEventData->OptionalObject);
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
  // Use consistent ASC retrieval for both players and enemies
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent(ActorInfo);

  if (ASC && DamageGameplayEffect) {
    // Set damage attribute on source for execution calculation
    ASC->SetNumericAttributeBase(UDamageAttributeSet::GetDamageAttribute(),
                                 Damage);

    FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(
        DamageGameplayEffect, 1.0f, ASC->MakeEffectContext());
    if (DamageSpecHandle.IsValid()) {
      ASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), ASC);
    }
  }

  ApplyDamageEffects(Damage, ImpactPoint, DamageDirection);

  // Check if character should die
  const UHealthAttributeSet *ConstAttributeSet = Cast<UHealthAttributeSet>(
      ASC->GetAttributeSet(UHealthAttributeSet::StaticClass()));
  UHealthAttributeSet *AttributeSet =
      const_cast<UHealthAttributeSet *>(ConstAttributeSet);
  if (AttributeSet) {
    float CurrentHealth = AttributeSet->GetHealth();
    if (CurrentHealth <= 0.0f) {
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
        ACombatBase *CombatBase = GetCombatBaseFromActorInfo();
        if (CombatBase) {
          CombatBase->ReceivedDamage(Damage, ImpactPoint, DamageDirection);

          if (Cast<ACombatEnemy>(CombatBase)) {
            // stop the attack montages to interrupt the attack
            if (UAnimInstance *AnimInstance = Cast<ACombatEnemy>(CombatBase)
                                                  ->GetMesh()
                                                  ->GetAnimInstance()) {
              AnimInstance->Montage_Stop(
                  0.1f,
                  Cast<ACombatEnemy>(CombatBase)->GetComboAttackMontage());
              AnimInstance->Montage_Stop(
                  0.1f,
                  Cast<ACombatEnemy>(CombatBase)->GetChargedAttackMontage());
            }
          }
        }
      }

      EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
  }
}

void UCombatReceiveDamageAbility::ApplyDamageEffects(
    float Damage, const FVector &ImpactPoint, const FVector &DamageDirection) {
  if (!CurrentActorInfo->AvatarActor.IsValid()) {
    return;
  }

  ACombatBase *Character = GetCombatBaseFromActorInfo();
  if (!Character) {
    return;
  }

  // Apply knockback but keep the pelvis vertical
  Character->GetMesh()->SetPhysicsBlendWeight(0.5f);
  Character->GetMesh()->SetBodySimulatePhysics(Character->GetPelvisBoneName(),
                                               false);

  // apply knockback impulse
  Character->GetCharacterMovement()->AddImpulse(DamageDirection, true);
  // is the character ragdolling?
  if (Character->GetMesh()->IsSimulatingPhysics()) {
    // apply an impulse to the ragdoll
    Character->GetMesh()->AddImpulseAtLocation(
        DamageDirection * Character->GetMesh()->GetMass(), ImpactPoint);
  }
}
