// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagsManager.h"
#include "GameplayEffect.h"
#include "CombatPawnData.generated.h"

class UCombatAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

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
  TSubclassOf<UCombatAttributeSet> AttributeSet;

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

  /** Default trace distance */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultTraceDistance = 75.0f;

  /** Default trace radius */
  UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ClampMin = 0))
  float DefaultTraceRadius = 75.0f;

  /** Granted abilities */
  UPROPERTY(EditDefaultsOnly, Category = "Abilities")
  TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

  /** Granted effects */
  UPROPERTY(EditDefaultsOnly, Category = "Effects")
  TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

  /** Load attributes from data table */
  void LoadFromDataTable();
};
