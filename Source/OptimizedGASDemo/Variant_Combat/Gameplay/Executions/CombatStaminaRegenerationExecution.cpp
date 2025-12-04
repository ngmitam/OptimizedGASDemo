// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaRegenerationExecution.h"
#include "Attributes/StaminaAttributeSet.h"
#include "AbilitySystemComponent.h"

UCombatStaminaRegenerationExecution::UCombatStaminaRegenerationExecution() {
  // Capture stamina and max stamina attributes
  FGameplayEffectAttributeCaptureDefinition StaminaCapture(
      UStaminaAttributeSet::GetStaminaAttribute(),
      EGameplayEffectAttributeCaptureSource::Target, false);
  RelevantAttributesToCapture.Add(StaminaCapture);

  FGameplayEffectAttributeCaptureDefinition MaxStaminaCapture(
      UStaminaAttributeSet::GetMaxStaminaAttribute(),
      EGameplayEffectAttributeCaptureSource::Target, false);
  RelevantAttributesToCapture.Add(MaxStaminaCapture);
}

void UCombatStaminaRegenerationExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters &ExecutionParams,
    FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const {

  const FGameplayEffectSpec &Spec = ExecutionParams.GetOwningSpec();

  // Get the ability system component
  UAbilitySystemComponent *ASC =
      ExecutionParams.GetTargetAbilitySystemComponent();

  // Get current stamina and max stamina
  float CurrentStamina = 0.0f;
  FGameplayEffectAttributeCaptureDefinition StaminaCapture(
      UStaminaAttributeSet::GetStaminaAttribute(),
      EGameplayEffectAttributeCaptureSource::Target, false);
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
      StaminaCapture, FAggregatorEvaluateParameters(), CurrentStamina);

  float MaxStamina = 0.0f;
  FGameplayEffectAttributeCaptureDefinition MaxStaminaCapture(
      UStaminaAttributeSet::GetMaxStaminaAttribute(),
      EGameplayEffectAttributeCaptureSource::Target, false);
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
      MaxStaminaCapture, FAggregatorEvaluateParameters(), MaxStamina);

  // Don't regenerate if already at max
  if (CurrentStamina >= MaxStamina) {
    return;
  }

  // Don't regenerate if attacking
  if (ASC->HasMatchingGameplayTag(
          FGameplayTag::RequestGameplayTag(FName("State.Attacking")))) {
    return;
  }

  // Get the regeneration amount from SetByCaller
  float RegenAmount = Spec.GetSetByCallerMagnitude(
      FGameplayTag::RequestGameplayTag(FName("Data.StaminaRegenRate")), true,
      0.0f);

  // Calculate new stamina, but don't exceed max
  float NewStamina = FMath::Min(CurrentStamina + RegenAmount, MaxStamina);

  // Output the change
  OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
      UStaminaAttributeSet::GetStaminaAttribute(),
      EGameplayModOp::Override, // Use Override to set to exact value
      NewStamina));
}
