// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "CombatChargedAttackAbility.generated.h"

/**
 * GameplayAbility for performing charged attacks
 */
UCLASS()
class UCombatChargedAttackAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatChargedAttackAbility();

  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

  virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                          const FGameplayAbilityActorInfo *ActorInfo,
                          const FGameplayAbilityActivationInfo ActivationInfo,
                          bool bReplicateEndAbility,
                          bool bWasCancelled) override;

  /** Handle charged attack loop event */
  void HandleChargedAttackLoop(const FGameplayEventData *EventData);

  /** Handle charged attack release event */
  void HandleChargedAttackRelease(const FGameplayEventData *EventData);

  /** Called when montage ends */
  void OnMontageEnded(UAnimMontage *Montage, bool bInterrupted);

  /** Stamina costs for charged attack (per loop) */
  UPROPERTY(EditAnywhere, Category = "Charged Attack")
  TArray<float> StaminaCosts = {10.0f};

  /** Effect class to use for stamina cost */
  UPROPERTY(EditAnywhere, Category = "Charged Attack")
  TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

  /** Effect class to use for setting stamina used */
  UPROPERTY(EditAnywhere, Category = "Charged Attack")
  TSubclassOf<UGameplayEffect> StaminaUsedEffectClass;

protected:
  /** Montage to play for charged attack */
  UPROPERTY(EditDefaultsOnly, Category = "Charged Attack")
  UAnimMontage *ChargedAttackMontage;

  /** Section name for charge loop */
  UPROPERTY(EditDefaultsOnly, Category = "Charged Attack")
  FName ChargeLoopSection;

  /** Section name for charge attack */
  UPROPERTY(EditDefaultsOnly, Category = "Charged Attack")
  FName ChargeAttackSection;

  /** Montage ended delegate */
  FOnMontageEnded MontageEndedDelegate;

  /** Whether this ability is controlled by player or AI */
  bool bIsPlayerControlled;

private:
  /** Current charge loop count (for AI) */
  int32 CurrentChargeLoop = 0;

  /** Target charge loops (for AI) */
  int32 TargetChargeLoops = 0;

  /** Whether the ability has been auto-released due to insufficient stamina */
  bool bAutoReleased = false;

  /** Total stamina used in this charged attack (for damage scaling) */
  float TotalStaminaUsed = 0.0f;
};
