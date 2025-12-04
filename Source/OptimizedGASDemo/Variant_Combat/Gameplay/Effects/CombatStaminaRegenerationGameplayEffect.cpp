// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaRegenerationGameplayEffect.h"
#include "Attributes/StaminaAttributeSet.h"

UCombatStaminaRegenerationGameplayEffect::
    UCombatStaminaRegenerationGameplayEffect() {
  // Set duration policy to infinite (continuous regeneration)
  DurationPolicy = EGameplayEffectDurationType::HasDuration;
  DurationMagnitude = FGameplayEffectModifierMagnitude(0.0f); // Infinite

  // Set periodic execution every 0.1 seconds
  Period = 0.1f;
  bExecutePeriodicEffectOnApplication =
      false; // Don't execute on application to avoid magnitude issues

  // Modifiers: Increase stamina by rate * period per period
  FGameplayModifierInfo StaminaRegenModifier;
  StaminaRegenModifier.Attribute = UStaminaAttributeSet::GetStaminaAttribute();
  StaminaRegenModifier.ModifierOp = EGameplayModOp::Additive;
  // Use SetByCaller for dynamic amount per period
  FSetByCallerFloat SetByCallerFloat;
  SetByCallerFloat.DataTag =
      FGameplayTag::RequestGameplayTag(FName("Data.StaminaRegenRate"));
  FGameplayEffectModifierMagnitude SetByCallerMagnitude(SetByCallerFloat);
  StaminaRegenModifier.ModifierMagnitude = SetByCallerMagnitude;
  Modifiers.Add(StaminaRegenModifier);
}
