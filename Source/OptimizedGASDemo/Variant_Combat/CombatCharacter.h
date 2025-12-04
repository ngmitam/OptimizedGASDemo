// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "Animation/AnimInstance.h"
#include "Health/CombatHealthComponent.h"
#include "Data/CombatPawnData.h"
#include "EnhancedInputComponent.h"
#include "Data/CombatDamageEventData.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatNotifyEnemiesAbility.h"
#include "LockSystem/CombatLockSystemComponent.h"
#include "CombatCharacter.generated.h"

/**
 *  An enhanced Third Person Character with melee combat capabilities:
 *  - Combo attack string
 *  - Press and hold charged attack
 *  - Damage dealing and reaction
 *  - Death
 *  - Respawning
 */
UCLASS() class ACombatCharacter : public ACombatBase {
  GENERATED_BODY()

  /** Camera boom positioning the camera behind the character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  USpringArmComponent *CameraBoom;

  /** Follow camera */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UCameraComponent *FollowCamera;

  /** Lock system component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UCombatLockSystemComponent *LockSystemComponent;

protected:
  /** Jump Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *JumpAction;

  /** Move Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *MoveAction;

  /** Look Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *LookAction;

  /** Mouse Look Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *MouseLookAction;

  /** Combo Attack Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *ComboAttackAction;

  /** Charged Attack Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *ChargedAttackAction;

  /** Toggle Camera Side Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *ToggleCameraAction;

  /** Lock Target Input Action */
  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *LockAction;

  /** Life bar widget fill color */
  UPROPERTY(EditAnywhere, Category = "Damage")
  FLinearColor LifeBarColor;

  /** Camera boom length while the character is dead */
  UPROPERTY(EditAnywhere, Category = "Camera",
            meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
  float DeathCameraDistance = 400.0f;

  /** Camera boom length when the character respawns */
  UPROPERTY(EditAnywhere, Category = "Camera",
            meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
  float DefaultCameraDistance = 100.0f;

  /** Time to wait before respawning the character */
  UPROPERTY(EditAnywhere, Category = "Respawn",
            meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
  float RespawnTime = 3.0f;

  /** Character respawn timer */
  FTimerHandle RespawnTimer;

  /** Copy of the mesh's transform so we can reset it after ragdoll animations
   */
  FTransform MeshStartingTransform;

  /** Max amount of time that may elapse for a non-combo attack input to not be
   * considered stale */
  UPROPERTY(EditAnywhere, Category = "Melee Attack",
            meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
  float AttackInputCacheTimeTolerance = 2.0f;

  /** Camera shake for attacks */
  UPROPERTY(EditAnywhere, Category = "Feedback")
  TSubclassOf<UCameraShakeBase> AttackCameraShake;

  /** Time at which lock button was last pressed */
  float LastLockPressTime = 0.0f;

  /** Whether the camera is currently locked to a target */
  bool bCameraLocked = false;

  /** Interpolation speed for camera lock transitions */
  float CameraLockInterpSpeed = 5.0f;

  /** Stored velocity before attack for momentum preservation */
  FVector StoredVelocity = FVector::ZeroVector;

  /** Multiplier for momentum preservation during attacks */
  UPROPERTY(EditAnywhere, Category = "Movement|Combat")
  float AttackMomentumMultiplier = 0.7f;

  /** Original max walk speed before stun */
  float OriginalMaxWalkSpeed = 0.0f;

  /** Input smoothing factor for responsive controls */
  UPROPERTY(EditAnywhere, Category = "Input")
  float InputSmoothingFactor = 0.8f;

  /** Input deadzone to prevent accidental inputs */
  UPROPERTY(EditAnywhere, Category = "Input")
  float InputDeadzone = 0.1f;

  /** Smoothed input vector for movement */
  FVector2D SmoothedInput = FVector2D::ZeroVector;

  /** Camera boom length during combat attacks */
  UPROPERTY(EditAnywhere, Category = "Camera|Combat")
  float CombatCameraDistance = 250.0f;

  /** Camera boom length during exploration */
  UPROPERTY(EditAnywhere, Category = "Camera|Combat")
  float ExplorationCameraDistance = 400.0f;

protected:
public:
  ACombatCharacter();

  // ~begin IAbilitySystemInterface
  virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;
  // ~end IAbilitySystemInterface

  virtual void PossessedBy(AController *NewController) override;

  /** Get danger trace distance */
  float GetDangerTraceDistance() const { return DangerTraceDistance; }

  /** Get danger trace radius */
  float GetDangerTraceRadius() const { return DangerTraceRadius; }

  /** Get charged attack montage */
  UAnimMontage *GetChargedAttackMontage() const { return ChargedAttackMontage; }

  /** Get charge loop section name */
  FName GetChargeLoopSection() const { return ChargeLoopSection; }

  /** Get charge attack section name */
  FName GetChargeAttackSection() const { return ChargeAttackSection; }

protected:
  /** Called for movement input */
  void Move(const FInputActionValue &Value);

  /** Called for look input */
  void Look(const FInputActionValue &Value);

  /** Bind inputs from the pawn's InputConfig data asset (data-driven bindings)
   */
  void BindInputsFromConfig(UEnhancedInputComponent *EnhancedInputComponent);

  /** Bind a minimal set of movement/look inputs (fallback) */
  void BindBasicMovementInputs(UEnhancedInputComponent *EnhancedInputComponent);

  /** Send a gameplay event trigger tag to the ASC (bound from input mappings)
   */
  void SendAbilityTrigger(FGameplayTag TriggerTag);

  /** Called for toggle camera side input */
  void ToggleCamera();

  /** Called for lock target input */
  void LockPressed();

  /** Update camera lock to target */
  void UpdateCameraLock(float DeltaTime);

  /** Handle attack montage ended to execute buffered attacks */
  virtual void AttackMontageEnded(UAnimMontage *Montage,
                                  bool bInterrupted) override;

  /** BP hook to animate the camera side switch */
  UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
  void BP_ToggleCamera();

public:
  /** Handles move inputs from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoMove(float Right, float Forward);

  /** Handles look inputs from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoLook(float Yaw, float Pitch);

protected:
  // ~begin CombatAttacker interface

  /** Notifies nearby enemies that an attack is coming so they can react */
  void NotifyEnemiesOfAttack();

  // ~end CombatAttacker interface

public:
  /** Called from the respawn timer to destroy and re-create the character */
  void RespawnCharacter();

  /** Overrides landing to reset damage ragdoll physics */
  virtual void Landed(const FHitResult &Hit) override;

protected:
  /** Initialization */
  virtual void BeginPlay() override;

  /** Cleanup */
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  /** Handles input bindings */
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

  /** Handles possessed initialization */
  virtual void NotifyControllerChanged() override;

public:
  /** Returns CameraBoom subobject **/
  FORCEINLINE class USpringArmComponent *GetCameraBoom() const {
    return CameraBoom;
  }

  /** Returns FollowCamera subobject **/
  FORCEINLINE class UCameraComponent *GetFollowCamera() const {
    return FollowCamera;
  }

  /** Returns LockSystemComponent subobject **/
  FORCEINLINE class UCombatLockSystemComponent *GetLockSystemComponent() const {
    return LockSystemComponent;
  }

  /** Get attack camera shake class (public accessor for abilities) */
  TSubclassOf<UCameraShakeBase> GetAttackCameraShake() const {
    return AttackCameraShake;
  }

  /** Tick function */
  virtual void Tick(float DeltaTime) override;

  /** Check if has a locked target */
  bool HasLockedTarget() const;

  /** Get stored velocity for momentum preservation */
  const FVector &GetStoredVelocity() const { return StoredVelocity; }

  /** Get attack momentum multiplier */
  float GetAttackMomentumMultiplier() const { return AttackMomentumMultiplier; }

private:
  /** Handle movement speed attribute changes */
  void HandleMovementSpeedChanged(const FOnAttributeChangeData &ChangeData);
};
