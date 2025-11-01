// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
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

protected:
  /** The ability system component for this player state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UAbilitySystemComponent *AbilitySystemComponent;

  /** The attribute set for this player state */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UAttributeSet *AttributeSet;

  /** Default max HP for the player */
  UPROPERTY(EditAnywhere, Category = "Attributes",
            meta = (ClampMin = 0, ClampMax = 100))
  float DefaultMaxHP = 100.0f;

  /** Default melee damage */
  UPROPERTY(EditAnywhere, Category = "Attributes",
            meta = (ClampMin = 0, ClampMax = 100))
  float DefaultMeleeDamage = 1.0f;

  /** Default melee knockback impulse */
  UPROPERTY(EditAnywhere, Category = "Attributes",
            meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
  float DefaultMeleeKnockbackImpulse = 250.0f;

  /** Default melee launch impulse */
  UPROPERTY(EditAnywhere, Category = "Attributes",
            meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
  float DefaultMeleeLaunchImpulse = 300.0f;

protected:
  virtual void BeginPlay() override;
};
