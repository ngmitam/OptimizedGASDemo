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
    UAbilitySystemComponent *ASC) {
  if (!ASC) {
    return;
  }

  // Remove abilities
  for (const FGameplayAbilitySpecHandle &Handle : AbilitySpecHandles) {
    if (Handle.IsValid()) {
      ASC->ClearAbility(Handle);
    }
  }

  // Remove effects
  for (const FActiveGameplayEffectHandle &Handle : GameplayEffectHandles) {
    if (Handle.IsValid()) {
      ASC->RemoveActiveGameplayEffect(Handle);
    }
  }

  // Note: Attribute sets are typically permanent and not removed during runtime
  // They are managed by the AbilitySystemComponent's lifetime

  Reset();
}

bool FCombatAbilitySetHandle::IsValid() const {
  return AbilitySpecHandles.Num() > 0 || GameplayEffectHandles.Num() > 0 ||
         GrantedAttributeSets.Num() > 0;
}

void FCombatAbilitySetHandle::Reset() {
  AbilitySpecHandles.Reset();
  GameplayEffectHandles.Reset();
  GrantedAttributeSets.Reset();
}

UCombatAbilitySet::UCombatAbilitySet() {}

void UCombatAbilitySet::GiveToAbilitySystem(
    UAbilitySystemComponent *ASC, FCombatAbilitySetHandle &OutAbilitySetHandle,
    UObject *SourceObject) const {
  if (!ASC) {
    return;
  }

  // Grant abilities from the base GameplayAbilitySet
  for (const FGameplayAbilityBindInfo &BindInfo : Abilities) {
    if (BindInfo.GameplayAbilityClass) {
      // Check if ability is already granted
      if (!ASC->FindAbilitySpecFromClass(BindInfo.GameplayAbilityClass)) {
        FGameplayAbilitySpec AbilitySpec(BindInfo.GameplayAbilityClass, 1,
                                         (int32)BindInfo.Command);
        FGameplayAbilitySpecHandle AbilitySpecHandle =
            ASC->GiveAbility(AbilitySpec);
        if (AbilitySpecHandle.IsValid()) {
          OutAbilitySetHandle.AddAbilitySpecHandle(AbilitySpecHandle);
        }
      }
    }
  }

  // Grant attribute sets
  for (const FGameplayAttributeApplicationInfo &AttributeInfo :
       GrantedAttributes) {
    if (AttributeInfo.AttributeSet) {
      // Check if attribute set is already added
      if (!ASC->GetAttributeSet(AttributeInfo.AttributeSet)) {
        UAttributeSet *NewAttributeSet = NewObject<UAttributeSet>(
            ASC->GetOwner(), AttributeInfo.AttributeSet);
        if (NewAttributeSet) {
          ASC->AddAttributeSetSubobject(NewAttributeSet);
          OutAbilitySetHandle.AddAttributeSet(NewAttributeSet);
        }
      }
    }
  }

  // Grant gameplay effects
  for (const FGameplayEffectApplicationInfo &EffectInfo :
       GrantedGameplayEffects) {
    if (EffectInfo.GameplayEffect) {
      // Check if effect is already active
      FGameplayEffectQuery Query;
      Query.EffectDefinition = EffectInfo.GameplayEffect;
      TArray<FActiveGameplayEffectHandle> ActiveEffects =
          ASC->GetActiveEffects(Query);
      if (ActiveEffects.Num() == 0) {
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
            EffectInfo.GameplayEffect, 1.0f, ASC->MakeEffectContext());
        if (SpecHandle.IsValid()) {
          FActiveGameplayEffectHandle EffectHandle =
              ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
          if (EffectHandle.IsValid()) {
            OutAbilitySetHandle.AddGameplayEffectHandle(EffectHandle);
          }
        }
      }
    }
  }
}

void UCombatAbilitySet::TakeFromAbilitySystem(
    UAbilitySystemComponent *ASC, FCombatAbilitySetHandle &AbilitySetHandle) {
  if (ASC && AbilitySetHandle.IsValid()) {
    AbilitySetHandle.TakeFromAbilitySystem(ASC);
  }
}
