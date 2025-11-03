// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "../Data/DamageEventData.h"
#include "CombatReceiveDamageAbility.generated.h"

/**
 * GameplayAbility for receiving damage
 */
UCLASS()
class UCombatReceiveDamageAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatReceiveDamageAbility();

  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

protected:
  /** Camera shake for damage */
  UPROPERTY(EditDefaultsOnly, Category = "Damage")
  TSubclassOf<UCameraShakeBase> DamageCameraShake;

  /** Camera shake for death */
  UPROPERTY(EditDefaultsOnly, Category = "Damage")
  TSubclassOf<UCameraShakeBase> DeathCameraShake;

  /** Apply damage effects */
  void ApplyDamageEffects(float Damage, const FVector &ImpactPoint,
                          const FVector &DamageDirection);
};
