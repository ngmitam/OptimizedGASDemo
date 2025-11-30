// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaCostGameplayEffect.h"
#include "Attributes/StaminaAttributeSet.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"

UCombatStaminaCostGameplayEffect::UCombatStaminaCostGameplayEffect() {
  // Set duration policy to instant
  DurationPolicy = EGameplayEffectDurationType::Instant;

  // Modifiers: Reduce stamina by caller-specified amount
  FGameplayModifierInfo StaminaModifier;
  StaminaModifier.Attribute = UStaminaAttributeSet::GetStaminaAttribute();
  StaminaModifier.ModifierOp = EGameplayModOp::Additive;
  FSetByCallerFloat SetByCallerFloat;
  SetByCallerFloat.DataTag =
      FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost"));
  FGameplayEffectModifierMagnitude SetByCallerMagnitude(SetByCallerFloat);
  StaminaModifier.ModifierMagnitude = SetByCallerMagnitude;
  Modifiers.Add(StaminaModifier);
}

void UCombatStaminaCostGameplayEffect::PostInitProperties() {
  Super::PostInitProperties();
}
