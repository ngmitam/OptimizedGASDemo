// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "Animation/AnimInstance.h"
#include "HealthComponent.h"
#include "Gameplay/Attributes/CombatAttributeSet.h"
#include "Gameplay/Data/CombatPawnData.h"
#include "Gameplay/Data/DamageEventData.h"
#include "CombatBase.generated.h"

class UWidgetComponent;
class UCombatLifeBar;
class UAnimMontage;

/**
 * Base class for combat characters and enemies with shared combat functionality
 */
UCLASS(abstract)
class ACombatBase : public ACharacter,
                    public IAbilitySystemInterface,
                    public ICombatAttacker,
                    public ICombatDamageable {
  GENERATED_BODY()

protected:
  /** Life bar widget component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UWidgetComponent *LifeBar;

  /** Health component for managing health */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UHealthComponent *HealthComponent;

  /** The ability system component for this character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UAbilitySystemComponent *AbilitySystemComponent;

  /** The attribute set for this character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS",
            meta = (AllowPrivateAccess = "true"))
  UCombatAttributeSet *AttributeSet;

  /** Pawn data for attributes and abilities */
  UPROPERTY(EditDefaultsOnly, Category = "GAS")
  UCombatPawnData *PawnData;
  /** Name of the pelvis bone, for damage ragdoll physics */
  UPROPERTY(EditAnywhere, Category = "Damage")
  FName PelvisBoneName;

  /** Pointer to the life bar widget */
  UPROPERTY(EditAnywhere, Category = "Damage")
  TObjectPtr<UCombatLifeBar> LifeBarWidget;

  /** Max amount of HP the character will have */
  UPROPERTY(EditAnywhere, Category = "Damage")
  float MaxHP = 100.0f;

  /** Current amount of HP the character has */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage",
            meta = (ClampMin = 0))
  float CurrentHP = 100.0f;

  /** Amount of damage a melee attack will deal */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Damage",
            meta = (ClampMin = 0, ClampMax = 100))
  float MeleeDamage = 5.0f;

  /** Amount of knockback impulse a melee attack will apply */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Damage",
            meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
  float MeleeKnockbackImpulse = 500.0f;

  /** Amount of upwards impulse a melee attack will apply */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Damage",
            meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
  float MeleeLaunchImpulse = 300.0f;

  /** Index of the current stage of the melee attack combo */
  int32 ComboCount = 0;

  /** Target number of attacks in the combo attack string we're playing */
  int32 TargetComboCount = 0;

  /** Index of the current stage of the melee attack combo */
  int32 CurrentComboAttack = 0;

  /** Flag that determines if the character is currently holding the charged
   * attack input */
  bool bIsChargingAttack = false;

  /** If true, the charged attack hold check has been tested at least once */
  bool bHasLoopedChargedAttack = false;

  /** Time to wait before removing this character from the level after it dies
   */
  UPROPERTY(EditAnywhere, Category = "Death")
  float DeathRemovalTime = 5.0f;

  /** Character death timer */
  FTimerHandle DeathTimer;

  /** Last recorded location we're being attacked from */
  FVector LastDangerLocation = FVector::ZeroVector;

  /** Last recorded game time we were attacked */
  float LastDangerTime = -1000.0f;

  /** Distance ahead of the character that enemies will be notified of incoming
   * attacks */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Trace",
            meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
  float DangerTraceDistance = 300.0f;

  /** Radius of the sphere trace to notify enemies of incoming attacks */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Trace",
            meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
  float DangerTraceRadius = 100.0f;

  /** If true, the character is currently playing an attack animation */
  bool bIsAttacking = false;

  /** Distance ahead of the character that melee attack sphere collision traces
   * will extend */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Trace",
            meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
  float MeleeTraceDistance = 75.0f;

  /** Radius of the sphere trace for melee attacks */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Trace",
            meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
  float MeleeTraceRadius = 75.0f;

  /** AnimMontage that will play for combo attacks */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Combo")
  UAnimMontage *ComboAttackMontage;

  /** Names of the AnimMontage sections that correspond to each stage of the
   * combo attack */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Combo")
  TArray<FName> ComboSectionNames;

  /** AnimMontage that will play for charged attacks */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Charged")
  UAnimMontage *ChargedAttackMontage;

  /** Name of the AnimMontage section that corresponds to the charge loop */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Charged")
  FName ChargeLoopSection;

  /** Name of the AnimMontage section that corresponds to the attack */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Charged")
  FName ChargeAttackSection;

  /** Attack montage ended delegate */
  FOnMontageEnded OnAttackMontageEnded;

public:
  /** Constructor */
  ACombatBase();

  // ~begin IAbilitySystemInterface
  virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;
  // ~end IAbilitySystemInterface

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

  /** Get attack montage ended delegate */
  FOnMontageEnded &GetOnAttackMontageEnded() { return OnAttackMontageEnded; }

  /** Get pelvis bone name */
  FName GetPelvisBoneName() const { return PelvisBoneName; }

  /** Get combo input cache time tolerance */
  virtual float GetComboInputCacheTimeTolerance() const { return 0.45f; }

  /** Get danger trace distance */
  float GetDangerTraceDistance() const { return DangerTraceDistance; }

  /** Get danger trace radius */
  float GetDangerTraceRadius() const { return DangerTraceRadius; }

  /** Get cached combo attack input time */
  virtual float GetCachedComboAttackInputTime() const { return 0.0f; }

  /** Set cached combo attack input time */
  virtual void SetCachedComboAttackInputTime(float Time) {}

  /** Get cached charged attack input time */
  virtual float GetCachedChargedAttackInputTime() const { return 0.0f; }

  /** Set cached charged attack input time */
  virtual void SetCachedChargedAttackInputTime(float Time) {}

  /** Returns the last recorded location we were attacked from */
  const FVector &GetLastDangerLocation() const { return LastDangerLocation; }

  /** Returns the last game time we were attacked */
  float GetLastDangerTime() const { return LastDangerTime; }

  /** Set attacking flag */
  void SetIsAttacking(bool bAttacking) { bIsAttacking = bAttacking; }

protected:
  // ~begin CombatAttacker interface

  /** Performs the collision check for an attack */
  virtual void DoAttackTrace(FName DamageSourceBone) override;

  /** Performs the combo string check */
  virtual void CheckCombo() override;

  /** Performs the charged attack hold check */
  virtual void CheckChargedAttack() override;

  // ~end CombatAttacker interface

  // ~begin CombatDamageable interface

  /** Handles damage and knockback events */
  virtual void ApplyDamage(float Damage, AActor *DamageCauser,
                           const FVector &DamageLocation,
                           const FVector &DamageImpulse) override;

  /** Handles death events */
  virtual void HandleDeath() override;

  /** Handles healing events */
  virtual void ApplyHealing(float Healing, AActor *Healer) override;

  /** Notifies of danger */
  virtual void NotifyDanger(const FVector &DangerLocation,
                            AActor *DangerSource) override;

  // ~end CombatDamageable interface

protected:
  /** Initialization */
  virtual void BeginPlay() override;

  /** Cleanup */
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  /** Initialize ability system components */
  void InitializeAbilitySystemComponents();

  /** Initialize pawn data and grant abilities/effects */
  void InitializePawnData();

  /** Helper method to send gameplay events */
  void SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude = 0.0f,
                         UObject *OptionalObject = nullptr,
                         AActor *EventInstigator = nullptr);

private:
  /** Default effect level for granted effects */
  static constexpr float DefaultEffectLevel = 1.0f;

public:
  /** Overrides landing to reset damage ragdoll physics */
  virtual void Landed(const FHitResult &Hit) override;

  /** Called from a delegate when the attack montage ends */
  virtual void AttackMontageEnded(UAnimMontage *Montage, bool bInterrupted);

  /** Called when HealthComponent health changes */
  void OnHealthComponentChanged(float NewHealth);

  /** Called when HealthComponent max health changes */
  void OnMaxHealthComponentChanged(float NewMaxHealth);

  /** Resets the character's current HP to maximum */
  void ResetHP();

  /** Removes this character from the level after it dies */
  void RemoveFromLevel();

  /** Notifies nearby enemies that an attack is coming */
  void NotifyEnemiesOfAttack();

  /** Blueprint handler to play damage received effects */
  UFUNCTION(BlueprintNativeEvent, Category = "Combat")
  void ReceivedDamage(float Damage, const FVector &ImpactPoint,
                      const FVector &DamageDirection);

  /** Blueprint handler to play damage dealt effects */
  UFUNCTION(BlueprintNativeEvent, Category = "Combat")
  void DealtDamage(float Damage, const FVector &ImpactPoint);
};
