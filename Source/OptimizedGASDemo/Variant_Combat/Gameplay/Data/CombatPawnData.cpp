// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatPawnData.h"
#include "Abilities/GameplayAbility.h"
#include "CombatAttributeTableRow.h"

UCombatPawnData::UCombatPawnData() {}

void UCombatPawnData::LoadFromDataTable() {
  if (AttributeDataTable && !AttributeTableRowName.IsNone()) {
    FCombatAttributeTableRow *Row =
        AttributeDataTable->FindRow<FCombatAttributeTableRow>(
            AttributeTableRowName, TEXT("CombatPawnData"));
    if (Row) {
      DefaultHealth = Row->DefaultHealth;
      DefaultMaxHealth = Row->DefaultMaxHealth;
      DefaultDamage = Row->DefaultDamage;
      DefaultKnockbackImpulse = Row->DefaultKnockbackImpulse;
      DefaultLaunchImpulse = Row->DefaultLaunchImpulse;

      // Load abilities (legacy support)
      GrantedAbilities.Empty();
      for (TSoftClassPtr<UGameplayAbility> AbilityPtr : Row->GrantedAbilities) {
        if (TSubclassOf<UGameplayAbility> AbilityClass =
                AbilityPtr.LoadSynchronous()) {
          GrantedAbilities.Add(AbilityClass);
        } else {
          UE_LOG(LogTemp, Error, TEXT("Failed to load ability class: %s"),
                 *AbilityPtr.ToString());
        }
      }

      // Load effects (legacy support)
      GrantedEffects.Empty();
      for (TSoftClassPtr<UGameplayEffect> EffectPtr : Row->GrantedEffects) {
        if (TSubclassOf<UGameplayEffect> EffectClass =
                EffectPtr.LoadSynchronous()) {
          GrantedEffects.Add(EffectClass);
        } else {
          UE_LOG(LogTemp, Error, TEXT("Failed to load effect class: %s"),
                 *EffectPtr.ToString());
        }
      }
    } else {
      UE_LOG(LogTemp, Error, TEXT("Failed to load attribute data for row: %s"),
             *AttributeTableRowName.ToString());
    }
  }
}

TArray<TSubclassOf<UGameplayAbility>>
UCombatPawnData::GetAllGrantedAbilities() const {
  TArray<TSubclassOf<UGameplayAbility>> AllAbilities;

  // Add abilities from AbilitySets
  for (const FCombatAbilitySetWithInput &AbilitySetWithInput : AbilitySets) {
    if (UCombatAbilitySet *AbilitySet = AbilitySetWithInput.AbilitySet.Get()) {
      for (const FGameplayAbilityBindInfo &BindInfo : AbilitySet->Abilities) {
        if (BindInfo.GameplayAbilityClass) {
          AllAbilities.AddUnique(BindInfo.GameplayAbilityClass);
        }
      }
    }
  }

  // Add legacy abilities
  for (TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities) {
    if (AbilityClass) {
      AllAbilities.AddUnique(AbilityClass);
    }
  }

  return AllAbilities;
}

TArray<TSubclassOf<UGameplayEffect>>
UCombatPawnData::GetAllGrantedEffects() const {
  TArray<TSubclassOf<UGameplayEffect>> AllEffects;

  // Add effects from AbilitySets
  for (const FCombatAbilitySetWithInput &AbilitySetWithInput : AbilitySets) {
    if (UCombatAbilitySet *AbilitySet = AbilitySetWithInput.AbilitySet.Get()) {
      for (const FGameplayEffectApplicationInfo &EffectInfo :
           AbilitySet->GrantedGameplayEffects) {
        if (EffectInfo.GameplayEffect) {
          AllEffects.AddUnique(EffectInfo.GameplayEffect);
        }
      }
    }
  }

  // Add legacy effects
  for (TSubclassOf<UGameplayEffect> EffectClass : GrantedEffects) {
    if (EffectClass) {
      AllEffects.AddUnique(EffectClass);
    }
  }

  return AllEffects;
}
