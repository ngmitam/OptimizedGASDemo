// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "CombatTraceAttackAbility.generated.h"

class ACombatCharacter;

/**
 * GameplayAbility for performing attack traces
 */
UCLASS()
class UCombatTraceAttackAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatTraceAttackAbility();

  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

protected:
  /** Damage to apply */
  UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = 0))
  float DamageAmount = 10.0f;

  /** Knockback impulse */
  UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = 0))
  float KnockbackImpulse = 500.0f;

  /** Launch impulse */
  UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = 0))
  float LaunchImpulse = 300.0f;

  /** Trace distance for attack */
  UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = 0))
  float TraceDistance = 75.0f;

  /** Trace radius for attack */
  UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = 0))
  float TraceRadius = 75.0f;

  /** Multiplier for stamina used to damage scaling */
  UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = 0))
  float StaminaDamageMultiplier = 1.5f;

  /** Cached trigger event data */
  const FGameplayEventData *CachedTriggerEventData = nullptr;

  /** Perform the attack trace */
  void PerformAttackTrace();

  /** Returns the CombatCharacter that owns this ability */
  class ACombatCharacter *GetCombatCharacterFromActorInfo() const;
};
