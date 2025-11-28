// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void FCombatAbilitySetHandle::AddAbilitySpecHandle(
    const FGameplayAbilitySpecHandle &Handle) {
  AbilitySpecHandles.Add(Handle);
}

void FCombatAbilitySetHandle::AddGameplayEffectHandle(
    const FActiveGameplayEffectHandle &Handle) {
  GameplayEffectHandles.Add(Handle);
}

void FCombatAbilitySetHandle::AddAttributeSet(UAttributeSet *AttributeSet) {
  GrantedAttributeSets.Add(AttributeSet);
}

void FCombatAbilitySetHandle::TakeFromAbilitySystem(
    UAbilitySystemComponent *AbilitySystemComponent) {
  if (!AbilitySystemComponent) {
    return;
  }

  // Remove abilities
  for (const FGameplayAbilitySpecHandle &Handle : AbilitySpecHandles) {
    AbilitySystemComponent->ClearAbility(Handle);
  }

  // Remove effects
  for (const FActiveGameplayEffectHandle &Handle : GameplayEffectHandles) {
    AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
  }

  // Remove attribute sets
  for (UAttributeSet *AttributeSet : GrantedAttributeSets) {
    // Note: Attribute sets are typically permanent and not removed
    // AbilitySystemComponent->RemoveAttributeSetSubobject(AttributeSet);
  }

  AbilitySpecHandles.Reset();
  GameplayEffectHandles.Reset();
  GrantedAttributeSets.Reset();
}

UCombatAbilitySet::UCombatAbilitySet() {}

void UCombatAbilitySet::GiveToAbilitySystem(
    UAbilitySystemComponent *AbilitySystemComponent,
    FCombatAbilitySetHandle &AbilitySetHandle, UObject *SourceObject) const {
  if (!AbilitySystemComponent) {
    return;
  }

  // Grant abilities
  for (const FGameplayAbilityBindInfo &BindInfo : Abilities) {
    if (BindInfo.GameplayAbilityClass) {
      FGameplayAbilitySpecHandle AbilitySpecHandle =
          AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(
              BindInfo.GameplayAbilityClass, 1, (int32)BindInfo.Command));
      AbilitySetHandle.AddAbilitySpecHandle(AbilitySpecHandle);
    }
  }

  // Grant attribute sets
  for (const FGameplayAttributeApplicationInfo &AttributeInfo :
       GrantedAttributes) {
    if (AttributeInfo.AttributeSet) {
      UAttributeSet *NewAttributeSet = NewObject<UAttributeSet>(
          AbilitySystemComponent->GetOwner(), AttributeInfo.AttributeSet);
      AbilitySystemComponent->AddAttributeSetSubobject(NewAttributeSet);
      AbilitySetHandle.AddAttributeSet(NewAttributeSet);
    }
  }

  // Grant gameplay effects
  for (const FGameplayEffectApplicationInfo &EffectInfo :
       GrantedGameplayEffects) {
    if (EffectInfo.GameplayEffect) {
      FGameplayEffectSpecHandle SpecHandle =
          AbilitySystemComponent->MakeOutgoingSpec(
              EffectInfo.GameplayEffect, 1.0f, FGameplayEffectContextHandle());
      if (SpecHandle.IsValid()) {
        FActiveGameplayEffectHandle EffectHandle =
            AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
                *SpecHandle.Data.Get(), AbilitySystemComponent);
        AbilitySetHandle.AddGameplayEffectHandle(EffectHandle);
      }
    }
  }
}
