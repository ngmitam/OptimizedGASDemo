// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayEffect.h"
#include "CombatStaminaRegenerationGameplayEffect.generated.h"

/**
 * GameplayEffect for stamina regeneration over time
 */
UCLASS()
class UCombatStaminaRegenerationGameplayEffect : public UCombatGameplayEffect {
  GENERATED_BODY()

public:
  UCombatStaminaRegenerationGameplayEffect();

  /** The amount of stamina to regenerate per second */
  UPROPERTY(EditAnywhere, Category = "Stamina Regeneration")
  float StaminaRegenerationRate = 10.0f;
};
