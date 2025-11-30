// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaStunGameplayEffect.h"
#include "Attributes/MovementAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UCombatStaminaStunGameplayEffect::UCombatStaminaStunGameplayEffect() {
  // Set duration policy to while active (until stamina regenerates)
  DurationPolicy = EGameplayEffectDurationType::HasDuration;
  DurationMagnitude =
      FGameplayEffectModifierMagnitude(0.0f); // Infinite until removed

  // Create target tags component for granted tags
  UTargetTagsGameplayEffectComponent *TargetTagsComponent =
      CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(
          TEXT("TargetTags"));
  FInheritedTagContainer TagContainer;
  TagContainer.Added.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Stunned")));
  TargetTagsComponent->SetAndApplyTargetTagChanges(TagContainer);
  GEComponents.Add(TargetTagsComponent);

  // Modifier: Set movement speed to 0 for stun
  FGameplayModifierInfo MovementModifier;
  MovementModifier.Attribute =
      UMovementAttributeSet::GetMovementSpeedAttribute();
  MovementModifier.ModifierOp = EGameplayModOp::Override;
  MovementModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(0.0f);
  Modifiers.Add(MovementModifier);
}
