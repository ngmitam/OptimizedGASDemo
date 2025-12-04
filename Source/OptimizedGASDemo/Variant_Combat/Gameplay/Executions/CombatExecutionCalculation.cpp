// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatExecutionCalculation.h"
#include "Attributes/DamageAttributeSet.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Attributes/HealthAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CombatCharacter.h"
#include "CombatPlayerState.h"
#include "Gameplay/Data/CombatPawnData.h"

UCombatDamageExecutionCalculation::UCombatDamageExecutionCalculation() {
  // Capture Damage attribute from source
  RelevantAttributesToCapture.Add(FGameplayEffectAttributeCaptureDefinition(
      UDamageAttributeSet::GetDamageAttribute(),
      EGameplayEffectAttributeCaptureSource::Source, true));
}

void UCombatDamageExecutionCalculation::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters &ExecutionParams,
    FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const {
  const FGameplayEffectSpec &Spec = ExecutionParams.GetOwningSpec();

  // Get damage value from source
  float Damage = 0.0f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
      FGameplayEffectAttributeCaptureDefinition(
          UDamageAttributeSet::GetDamageAttribute(),
          EGameplayEffectAttributeCaptureSource::Source, true),
      FAggregatorEvaluateParameters(), Damage);

  // Get multiplier from PawnData
  float Multiplier = DamageMultiplier;
  if (ACombatCharacter *CombatChar =
          Cast<ACombatCharacter>(Spec.GetContext().GetInstigator())) {
    if (ACombatPlayerState *PlayerState =
            Cast<ACombatPlayerState>(CombatChar->GetPlayerState())) {
      if (const UCombatPawnData *PawnData = PlayerState->GetPawnData()) {
        Multiplier = PawnData->DamageMultiplier;
      }
    }
  }

  // Apply multiplier
  Damage *= Multiplier;

  // Output health reduction (damage is negative for health)
  OutExecutionOutput.AddOutputModifier(
      FGameplayModifierEvaluatedData(UHealthAttributeSet::GetHealthAttribute(),
                                     EGameplayModOp::Additive, -Damage));
}

UCombatStaminaExecutionCalculation::UCombatStaminaExecutionCalculation() {
  // Capture StaminaUsed attribute from source
  RelevantAttributesToCapture.Add(FGameplayEffectAttributeCaptureDefinition(
      UStaminaAttributeSet::GetStaminaUsedAttribute(),
      EGameplayEffectAttributeCaptureSource::Source, true));
}

void UCombatStaminaExecutionCalculation::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters &ExecutionParams,
    FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const {
  const FGameplayEffectSpec &Spec = ExecutionParams.GetOwningSpec();

  // Get stamina used value from source
  float StaminaUsed = 0.0f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
      FGameplayEffectAttributeCaptureDefinition(
          UStaminaAttributeSet::GetStaminaUsedAttribute(),
          EGameplayEffectAttributeCaptureSource::Source, true),
      FAggregatorEvaluateParameters(), StaminaUsed);

  // Get multiplier from PawnData
  float Multiplier = StaminaMultiplier;
  if (ACombatCharacter *CombatChar =
          Cast<ACombatCharacter>(Spec.GetContext().GetInstigator())) {
    if (ACombatPlayerState *PlayerState =
            Cast<ACombatPlayerState>(CombatChar->GetPlayerState())) {
      if (const UCombatPawnData *PawnData = PlayerState->GetPawnData()) {
        Multiplier = PawnData->StaminaMultiplier;
      }
    }
  }

  // Apply multiplier
  StaminaUsed *= Multiplier;

  // Output stamina reduction
  OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
      UStaminaAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive,
      -StaminaUsed));
}
