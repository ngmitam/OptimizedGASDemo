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

  /** For AI: minimum charge loops */
  UPROPERTY(EditDefaultsOnly, Category = "Charged Attack|AI",
            meta = (ClampMin = 1, ClampMax = 20))
  int32 MinChargeLoops = 2;

  /** For AI: maximum charge loops */
  UPROPERTY(EditDefaultsOnly, Category = "Charged Attack|AI",
            meta = (ClampMin = 1, ClampMax = 20))
  int32 MaxChargeLoops = 5;

  /** Current charge loop count */
  int32 CurrentChargeLoop = 0;

  /** Target charge loops (for AI) */
  int32 TargetChargeLoops = 0;

  /** Montage ended delegate */
  FOnMontageEnded MontageEndedDelegate;

  /** Whether this ability is controlled by player or AI */
  bool bIsPlayerControlled;
};
