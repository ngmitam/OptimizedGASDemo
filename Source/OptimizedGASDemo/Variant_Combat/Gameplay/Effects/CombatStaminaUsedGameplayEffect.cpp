// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaUsedGameplayEffect.h"
#include "Gameplay/Attributes/StaminaAttributeSet.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"

UCombatStaminaUsedGameplayEffect::UCombatStaminaUsedGameplayEffect() {
  // Set duration policy to instant
  DurationPolicy = EGameplayEffectDurationType::Instant;

  // Modifiers: Set stamina used to caller-specified amount
  FGameplayModifierInfo StaminaUsedModifier;
  StaminaUsedModifier.Attribute =
      UStaminaAttributeSet::GetStaminaUsedAttribute();
  StaminaUsedModifier.ModifierOp = EGameplayModOp::Override;
  FSetByCallerFloat SetByCallerFloat;
  SetByCallerFloat.DataTag =
      FGameplayTag::RequestGameplayTag(FName("Data.StaminaUsed"));
  FGameplayEffectModifierMagnitude SetByCallerMagnitude(SetByCallerFloat);
  StaminaUsedModifier.ModifierMagnitude = SetByCallerMagnitude;
  Modifiers.Add(StaminaUsedModifier);
}

void UCombatStaminaUsedGameplayEffect::PostInitProperties() {
  Super::PostInitProperties();
}
