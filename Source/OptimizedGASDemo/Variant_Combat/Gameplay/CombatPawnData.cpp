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
      DefaultTraceDistance = Row->DefaultTraceDistance;
      DefaultTraceRadius = Row->DefaultTraceRadius;

      // Load abilities
      GrantedAbilities.Empty();
      for (TSoftClassPtr<UGameplayAbility> AbilityPtr : Row->GrantedAbilities) {
        if (TSubclassOf<UGameplayAbility> AbilityClass =
                AbilityPtr.LoadSynchronous()) {
          GrantedAbilities.Add(AbilityClass);
        }
      }

      // Load effects
      GrantedEffects.Empty();
      for (TSoftClassPtr<UGameplayEffect> EffectPtr : Row->GrantedEffects) {
        if (TSubclassOf<UGameplayEffect> EffectClass =
                EffectPtr.LoadSynchronous()) {
          GrantedEffects.Add(EffectClass);
        }
      }
    }
  }
}
