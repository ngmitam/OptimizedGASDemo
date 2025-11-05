// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/CombatGameplayAbility.h"
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
};
