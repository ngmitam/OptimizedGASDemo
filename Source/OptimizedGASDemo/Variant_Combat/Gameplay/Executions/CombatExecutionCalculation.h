// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CombatExecutionCalculation.generated.h"

/**
 * Execution calculation for damage
 */
UCLASS(Blueprintable)
class UCombatDamageExecutionCalculation
    : public UGameplayEffectExecutionCalculation {
  GENERATED_BODY()

public:
  UCombatDamageExecutionCalculation();

  virtual void Execute_Implementation(
      const FGameplayEffectCustomExecutionParameters &ExecutionParams,
      FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const override;

  /** Damage multiplier */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
  float DamageMultiplier = 1.0f;
};

/**
 * Execution calculation for stamina usage
 */
UCLASS(Blueprintable)
class UCombatStaminaExecutionCalculation
    : public UGameplayEffectExecutionCalculation {
  GENERATED_BODY()

public:
  UCombatStaminaExecutionCalculation();

  virtual void Execute_Implementation(
      const FGameplayEffectCustomExecutionParameters &ExecutionParams,
      FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const override;

  /** Stamina cost multiplier */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
  float StaminaMultiplier = 1.0f;
};
