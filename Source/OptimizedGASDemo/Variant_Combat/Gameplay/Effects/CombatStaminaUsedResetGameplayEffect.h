// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayEffect.h"
#include "CombatStaminaUsedResetGameplayEffect.generated.h"

/**
 * GameplayEffect for resetting stamina used value to 0
 */
UCLASS()
class UCombatStaminaUsedResetGameplayEffect : public UCombatGameplayEffect {
  GENERATED_BODY()

public:
  UCombatStaminaUsedResetGameplayEffect();

  virtual void PostInitProperties() override;
};
