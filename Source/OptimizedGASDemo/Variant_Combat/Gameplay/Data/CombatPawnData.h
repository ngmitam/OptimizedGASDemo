// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "Gameplay/Abilities/CombatAbilitySet.h"
#include "CombatInputConfig.h"
#include "CombatPawnData.generated.h"

class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

/** Ability set with input mapping */
USTRUCT(BlueprintType)
struct FCombatAbilitySetWithInput {
  GENERATED_BODY()

  /** Ability set to grant */
  UPROPERTY(EditDefaultsOnly, Category = AbilitySet)
  TObjectPtr<UCombatAbilitySet> AbilitySet;
};

/**
 * Data asset for combat pawn attributes and abilities
 */
UCLASS()
class UCombatPawnData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UCombatPawnData();

  /** Data table row name to load attributes from */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes")
  FName AttributeTableRowName;

  /** Data table containing attribute data */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes")
  UDataTable *AttributeDataTable;

  /** Default attributes for this pawn */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes")
  TArray<TSubclassOf<UAttributeSet>> AttributeSets;

  /** Default health value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultHealth = 1000.0f;

  /** Default max health value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultMaxHealth = 1000.0f;

  /** Default damage value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultDamage = 10.0f;

  /** Default knockback impulse */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultKnockbackImpulse = 500.0f;

  /** Default launch impulse */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultLaunchImpulse = 300.0f;

  /** Default stamina value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultStamina = 100.0f;

  /** Default max stamina value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultMaxStamina = 100.0f;

  /** Granted ability sets with input mapping */
  UPROPERTY(EditDefaultsOnly, Category = "Abilities")
  TArray<FCombatAbilitySetWithInput> AbilitySets;

  /** Input configuration for this pawn */
  UPROPERTY(EditDefaultsOnly, Category = "Input")
  TObjectPtr<UCombatInputConfig> InputConfig;

  /** Damage multiplier for execution calculations */
  UPROPERTY(EditDefaultsOnly, Category = "Execution", meta = (ClampMin = 0))
  float DamageMultiplier = 1.0f;

  /** Stamina cost multiplier for execution calculations */
  UPROPERTY(EditDefaultsOnly, Category = "Execution", meta = (ClampMin = 0))
  float StaminaMultiplier = 1.0f;

  /** Load attributes from data table */
  void LoadFromDataTable();

  /** Get all granted abilities from AbilitySets*/
  TArray<TSubclassOf<UGameplayAbility>> GetAllGrantedAbilities() const;

  /** Get all granted effects from AbilitySets*/
  TArray<TSubclassOf<UGameplayEffect>> GetAllGrantedEffects() const;
};
