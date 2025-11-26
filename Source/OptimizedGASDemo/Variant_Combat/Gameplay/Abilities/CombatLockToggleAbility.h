// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/CombatGameplayAbility.h"
#include "CombatCharacter.h"
#include "CombatLockToggleAbility.generated.h"

/**
 * Ability to toggle lock on/off
 */
UCLASS()
class UCombatLockToggleAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatLockToggleAbility();

protected:
  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

  /** Server-side lock state toggle with validation */
  void Server_ToggleLockState(ACombatCharacter *CombatChar);

  /** Client-side prediction of lock toggle */
  void Client_PredictToggleLock(ACombatCharacter *CombatChar);
};
