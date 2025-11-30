// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
#include "Gameplay/Attributes/StaminaAttributeSet.h"
#include "Data/CombatPawnData.h"
#include "Gameplay/Abilities/CombatAbilitySet.h"
#include "CombatPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * PlayerState for Combat variant, includes AbilitySystemComponent
 */
UCLASS()
class ACombatPlayerState : public APlayerState, public IAbilitySystemInterface {
  GENERATED_BODY()

public:
  ACombatPlayerState();

  // ~begin IAbilitySystemInterface
  virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;
  // ~end IAbilitySystemInterface

  /** Set pawn data and grant abilities/effects */
  void SetPawnData(const UCombatPawnData *InPawnData);

  /** Clear all granted abilities/effects from previous pawn data */
  void ClearPawnData();

  /** Get default max HP */
  float GetDefaultMaxHP() const { return DefaultMaxHP; }

  /** Get default max stamina */
  float GetDefaultMaxStamina() const { return DefaultMaxStamina; }

protected:
  /** The ability system component for this player state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UAbilitySystemComponent *AbilitySystemComponent;

  /** The health attribute set for this player state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UHealthAttributeSet *HealthAttributeSet;

  /** The stamina attribute set for this player state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UStaminaAttributeSet *StaminaAttributeSet;

  /** The damage attribute set for this player state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UDamageAttributeSet *DamageAttributeSet;

  /** Default max HP for the player */
  UPROPERTY(EditAnywhere, Category = "Attributes",
            meta = (ClampMin = 0, ClampMax = 100))
  float DefaultMaxHP = 1000.0f;

  /** Default max Stamina for the player */
  UPROPERTY(EditAnywhere, Category = "Attributes",
            meta = (ClampMin = 0, ClampMax = 100))
  float DefaultMaxStamina = 100.0f;

protected:
  virtual void BeginPlay() override;

private:
  /** Handles for currently granted ability sets */
  TArray<FCombatAbilitySetHandle> GrantedAbilitySetHandles;

  /** Handles for legacy granted abilities (fallback when no AbilitySets
   * configured) */
  TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

  /** Handles for legacy granted effects (fallback when no AbilitySets
   * configured) */
  TArray<FActiveGameplayEffectHandle> GrantedEffectHandles;
};
