// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatDamageGameplayEffect.h"
#include "Attributes/CombatAttributeSet.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"

UCombatDamageGameplayEffect::UCombatDamageGameplayEffect() {
  // Set duration policy to instant
  DurationPolicy = EGameplayEffectDurationType::Instant;

  // Modifiers: Reduce health by damage amount using SetByCaller
  FGameplayModifierInfo HealthModifier;
  HealthModifier.Attribute = UCombatAttributeSet::GetHealthAttribute();
  HealthModifier.ModifierOp = EGameplayModOp::Additive;
  FGameplayTag DamageTag =
      FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
  FSetByCallerFloat SetByCaller;
  SetByCaller.DataTag = DamageTag;
  HealthModifier.ModifierMagnitude =
      FGameplayEffectModifierMagnitude(SetByCaller);
  Modifiers.Add(HealthModifier);
}
