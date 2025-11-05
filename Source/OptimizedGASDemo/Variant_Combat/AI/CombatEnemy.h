// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "Animation/AnimMontage.h"
#include "Engine/TimerHandle.h"
#include "Attributes/CombatAttributeSet.h"
#include "Data/CombatPawnData.h"
#include "Health/CombatHealthComponent.h"
#include "Data/CombatDamageEventData.h"
#include "GameFramework/Character.h"
#include "AI/CombatAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatNotifyEnemiesAbility.h"
#include "CombatEnemy.generated.h"

/** Completed attack animation delegate for StateTree */
DECLARE_DELEGATE(FOnEnemyAttackCompleted);

/** Landed delegate for StateTree */
DECLARE_DELEGATE(FOnEnemyLanded);

/** Enemy died delegate */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

/**
 *  An AI-controlled character with combat capabilities.
 *  Its bundled AI Controller runs logic through StateTree
 */
UCLASS()
class ACombatEnemy : public ACombatBase {
  GENERATED_BODY()

public:
  ACombatEnemy();
  /** Attack completed internal delegate to notify StateTree tasks */
  FOnEnemyAttackCompleted OnAttackCompleted;

  /** Landed internal delegate to notify StateTree tasks. We use this instead of
   * the built-in Landed delegate so we can bind to a Lambda in StateTree tasks
   */
  FOnEnemyLanded OnEnemyLanded;

  /** Enemy died delegate. Allows external subscribers to respond to enemy death
   */
  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnEnemyDied OnEnemyDied;

  /** Get combo attack montage */
  UAnimMontage *GetComboAttackMontage() const { return ComboAttackMontage; }

  /** Get combo section names */
  const TArray<FName> &GetComboSectionNames() const {
    return ComboSectionNames;
  }

  /** Get charged attack montage */
  UAnimMontage *GetChargedAttackMontage() const { return ChargedAttackMontage; }

  /** Get charge loop section name */
  FName GetChargeLoopSection() const { return ChargeLoopSection; }

  /** Get charge attack section name */
  FName GetChargeAttackSection() const { return ChargeAttackSection; }

  /** Get pelvis bone name */
  FName GetPelvisBoneName() const { return PelvisBoneName; }

  /** Get minimum charge loops */
  int32 GetMinChargeLoops() const { return MinChargeLoops; }

  /** Get maximum charge loops */
  int32 GetMaxChargeLoops() const { return MaxChargeLoops; }

  /** If true, the charged attack hold check has been tested at least once */
  bool bHasLoopedChargedAttack = false;

  /** Minimum number of charge animation loops that will be played by the AI */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Charged",
            meta = (ClampMin = 1, ClampMax = 20))
  int32 MinChargeLoops = 2;

  /** Maximum number of charge animation loops that will be played by the AI */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Charged",
            meta = (ClampMin = 1, ClampMax = 20))
  int32 MaxChargeLoops = 5;

  /** Target number of charge animation loops to play in this charged attack */
  int32 TargetChargeLoops = 0;

  /** Number of charge animation loop currently playing */
  int32 CurrentChargeLoop = 0;

public:
  /** Performs an AI-initiated combo attack. Number of hits will be decided by
   * this character */
  void DoAIComboAttack();

  /** Performs an AI-initiated charged attack. Charge time will be decided by
   * this character */
  void DoAIChargedAttack();

  /** Called from a delegate when the attack montage ends */
  virtual void AttackMontageEnded(UAnimMontage *Montage,
                                  bool bInterrupted) override;

public:
  // ~begin ICombatAttacker interface

  // ~end ICombatAttacker interface

  // ~begin ICombatDamageable interface

  // ~end ICombatDamageable interface

public:
  /** Overrides landing to reset damage ragdoll physics */
  virtual void Landed(const FHitResult &Hit) override;

protected:
  /** Gameplay initialization */
  virtual void BeginPlay() override;

  /** EndPlay cleanup */
  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  /** Handles death events */
  virtual void HandleDeath() override;
};
