// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaRegenerationGameplayEffect.h"
#include "Executions/CombatStaminaRegenerationExecution.h"

UCombatStaminaRegenerationGameplayEffect::
    UCombatStaminaRegenerationGameplayEffect() {
  // Set duration policy to infinite (continuous regeneration)
  DurationPolicy = EGameplayEffectDurationType::HasDuration;
  DurationMagnitude = FGameplayEffectModifierMagnitude(0.0f); // Infinite

  // Set periodic execution every 0.1 seconds
  Period = 0.1f;
  bExecutePeriodicEffectOnApplication = true; // Execute periodically

  // Add execution calculation
  FGameplayEffectExecutionDefinition ExecutionDef;
  ExecutionDef.CalculationClass =
      UCombatStaminaRegenerationExecution::StaticClass();
  Executions.Add(ExecutionDef);
}
