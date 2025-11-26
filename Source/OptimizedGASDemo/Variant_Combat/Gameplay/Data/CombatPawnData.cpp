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
        } else {
          UE_LOG(LogTemp, Error, TEXT("Failed to load ability class: %s"),
                 *AbilityPtr.ToString());
        }
      }

      // Load effects
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
