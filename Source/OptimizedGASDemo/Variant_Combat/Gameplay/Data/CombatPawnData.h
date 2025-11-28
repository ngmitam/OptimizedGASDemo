// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "CombatPawnData.generated.h"

class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UCombatAbilitySet;

/** Ability set with input mapping */
USTRUCT(BlueprintType)
struct FCombatAbilitySetWithInput {
  GENERATED_BODY()

  /** Ability set to grant */
  UPROPERTY(EditDefaultsOnly, Category = AbilitySet)
  TObjectPtr<UCombatAbilitySet> AbilitySet;

  /** Input actions to bind to abilities in this set */
  UPROPERTY(EditDefaultsOnly, Category = AbilitySet)
  TArray<TObjectPtr<UInputAction>> InputActions;
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
  float DefaultHealth = 100.0f;

  /** Default max health value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultMaxHealth = 100.0f;

  /** Default damage value */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultDamage = 10.0f;

  /** Default knockback impulse */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultKnockbackImpulse = 500.0f;

  /** Default launch impulse */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultLaunchImpulse = 300.0f;

  /** Granted abilities (legacy - use AbilitySets instead) */
  UPROPERTY(EditDefaultsOnly, Category = "Abilities (Legacy)")
  TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

  /** Granted effects (legacy - use AbilitySets instead) */
  UPROPERTY(EditDefaultsOnly, Category = "Effects (Legacy)")
  TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

  /** Granted ability sets with input mapping */
  UPROPERTY(EditDefaultsOnly, Category = "Abilities")
  TArray<FCombatAbilitySetWithInput> AbilitySets;

  /** Load attributes from data table */
  void LoadFromDataTable();
};
