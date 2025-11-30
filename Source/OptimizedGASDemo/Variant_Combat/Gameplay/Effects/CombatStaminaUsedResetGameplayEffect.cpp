// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaUsedResetGameplayEffect.h"
#include "Gameplay/Attributes/StaminaAttributeSet.h"
#include "GameplayEffect.h"

UCombatStaminaUsedResetGameplayEffect::UCombatStaminaUsedResetGameplayEffect() {
  // Set duration policy to instant
  DurationPolicy = EGameplayEffectDurationType::Instant;

  // Modifiers: Reset stamina used to 0
  FGameplayModifierInfo StaminaUsedModifier;
  StaminaUsedModifier.Attribute =
      UStaminaAttributeSet::GetStaminaUsedAttribute();
  StaminaUsedModifier.ModifierOp = EGameplayModOp::Override;
  FGameplayEffectModifierMagnitude FixedMagnitude(0.0f);
  StaminaUsedModifier.ModifierMagnitude = FixedMagnitude;
  Modifiers.Add(StaminaUsedModifier);
}

void UCombatStaminaUsedResetGameplayEffect::PostInitProperties() {
  Super::PostInitProperties();
}
