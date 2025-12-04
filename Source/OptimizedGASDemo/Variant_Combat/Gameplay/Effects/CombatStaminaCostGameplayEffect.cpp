// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaCostGameplayEffect.h"
#include "Attributes/StaminaAttributeSet.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"
#include "Executions/CombatExecutionCalculation.h"

UCombatStaminaCostGameplayEffect::UCombatStaminaCostGameplayEffect() {
  // Set duration policy to instant
  DurationPolicy = EGameplayEffectDurationType::Instant;

  // Use execution calculation for stamina cost
  FGameplayEffectExecutionDefinition ExecutionDef;
  ExecutionDef.CalculationClass =
      UCombatStaminaExecutionCalculation::StaticClass();
  Executions.Add(ExecutionDef);
}

void UCombatStaminaCostGameplayEffect::PostInitProperties() {
  Super::PostInitProperties();
}
