// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CombatStaminaRegenerationExecution.generated.h"

/**
 * Execution calculation for stamina regeneration that stops when at max
 */
UCLASS()
class UCombatStaminaRegenerationExecution
    : public UGameplayEffectExecutionCalculation {
  GENERATED_BODY()

public:
  UCombatStaminaRegenerationExecution();

  virtual void Execute_Implementation(
      const FGameplayEffectCustomExecutionParameters &ExecutionParams,
      FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const override;
};
