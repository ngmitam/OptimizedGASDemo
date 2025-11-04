// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/CombatGameplayAbility.h"
#include "CombatComboAttackAbility.generated.h"

class UAnimMontage;

/**
 * Ability for performing combo attacks using GAS
 */
UCLASS()
class UCombatComboAttackAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatComboAttackAbility();

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

  /** Handle combo next event */
  void HandleComboNext(const FGameplayEventData *EventData);

  /** Called when montage ends */
  void OnMontageEnded(UAnimMontage *Montage, bool bInterrupted);

protected:
  /** Current combo count */
  int32 CurrentComboCount = 0;

  /** Target combo count for AI */
  int32 TargetComboCount = 0;

  /** Combo attack montage */
  UAnimMontage *ComboAttackMontage = nullptr;

  /** Combo section names */
  TArray<FName> ComboSectionNames;

  /** Montage ended delegate */
  FOnMontageEnded MontageEndedDelegate;
};
