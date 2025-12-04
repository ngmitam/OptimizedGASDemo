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

  return AllEffects;
}
