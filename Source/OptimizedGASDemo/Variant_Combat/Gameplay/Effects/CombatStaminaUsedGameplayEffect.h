// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayEffect.h"
#include "CombatStaminaUsedGameplayEffect.generated.h"

/**
 * GameplayEffect for setting stamina used value
 */
UCLASS()
class UCombatStaminaUsedGameplayEffect : public UCombatGameplayEffect {
  GENERATED_BODY()

public:
  UCombatStaminaUsedGameplayEffect();

  /** The amount of stamina used to set */
  UPROPERTY(EditAnywhere, Category = "Stamina Used")
  float StaminaUsed = 0.0f;

  virtual void PostInitProperties() override;
};
