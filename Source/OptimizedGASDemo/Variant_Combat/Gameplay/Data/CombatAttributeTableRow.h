// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CombatAttributeTableRow.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/**
 * Row structure for combat attribute data table
 */
USTRUCT(BlueprintType)
struct FCombatAttributeTableRow : public FTableRowBase {
  GENERATED_BODY()

  /** Display name */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
  FText DisplayName;

  /** Default health value */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes",
            meta = (ClampMin = 0))
  float DefaultHealth = 100.0f;

  /** Default max health value */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes",
            meta = (ClampMin = 0))
  float DefaultMaxHealth = 100.0f;

  /** Default damage value */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes",
            meta = (ClampMin = 0))
  float DefaultDamage = 10.0f;

  /** Default knockback impulse */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes",
            meta = (ClampMin = 0))
  float DefaultKnockbackImpulse = 500.0f;

  /** Default launch impulse */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes",
            meta = (ClampMin = 0))
  float DefaultLaunchImpulse = 300.0f;

  /** Granted abilities */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
  TArray<TSoftClassPtr<UGameplayAbility>> GrantedAbilities;

  /** Granted effects */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
  TArray<TSoftClassPtr<UGameplayEffect>> GrantedEffects;
};
