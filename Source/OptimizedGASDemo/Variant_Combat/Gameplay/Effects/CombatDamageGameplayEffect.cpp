// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatDamageGameplayEffect.h"
#include "Attributes/HealthAttributeSet.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"
#include "Executions/CombatExecutionCalculation.h"

UCombatDamageGameplayEffect::UCombatDamageGameplayEffect() {
  // Set duration policy to instant
  DurationPolicy = EGameplayEffectDurationType::Instant;

  // Use execution calculation for damage
  FGameplayEffectExecutionDefinition ExecutionDef;
  ExecutionDef.CalculationClass =
      UCombatDamageExecutionCalculation::StaticClass();
  Executions.Add(ExecutionDef);
}
