// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayEffect.h"
#include "CombatStaminaCostGameplayEffect.generated.h"

/**
 * GameplayEffect for applying stamina cost
 */
UCLASS()
class UCombatStaminaCostGameplayEffect : public UCombatGameplayEffect {
  GENERATED_BODY()

public:
  UCombatStaminaCostGameplayEffect();

  /** The amount of stamina to deduct (positive value) */
  UPROPERTY(EditAnywhere, Category = "Stamina Cost")
  float StaminaCost = 10.0f;

  virtual void PostInitProperties() override;
};
