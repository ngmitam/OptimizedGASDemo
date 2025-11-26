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
#include "Attributes/CombatAttributeSet.h"
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
UCLASS()
class ACombatCharacter : public ACombatBase {
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

  /** Max amount of time that may elapse for a combo attack input to not be
   * considered stale */
  UPROPERTY(EditAnywhere, Category = "Melee Attack|Combo",
            meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
  float ComboInputCacheTimeTolerance = 0.45f;

  /** Time at which a combo attack button was last pressed */
  float CachedComboAttackInputTime = 0.0f;

  /** Attack buffer time in seconds */
  UPROPERTY(EditAnywhere, Category = "Input|Combat")
  float AttackBufferTime = 0.2f;

  /** Timer handle for attack buffer */
  FTimerHandle AttackBufferTimer;

  /** Whether attack input is buffered */
  bool bAttackBuffered = false;

  /** Buffered attack type (0=none, 1=combo, 2=charged) */
  int32 BufferedAttackType = 0;

  /** Camera shake for attacks */
  UPROPERTY(EditAnywhere, Category = "Feedback")
  TSubclassOf<UCameraShakeBase> AttackCameraShake;

  /** Time at which a charged attack button was last pressed */
  float CachedChargedAttackInputTime = 0.0f;

  /** Whether the camera is currently locked to a target */
  bool bCameraLocked = false;

  /** Interpolation speed for camera lock transitions */
  float CameraLockInterpSpeed = 5.0f;

  /** Stored velocity before attack for momentum preservation */
  FVector StoredVelocity = FVector::ZeroVector;

  /** Multiplier for momentum preservation during attacks */
  UPROPERTY(EditAnywhere, Category = "Movement|Combat")
  float AttackMomentumMultiplier = 0.7f;

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

  /** Get combo input cache time tolerance */
  float GetComboInputCacheTimeTolerance() const override {
    return ComboInputCacheTimeTolerance;
  }

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

  /** Get cached combo attack input time */
  float GetCachedComboAttackInputTime() const {
    return CachedComboAttackInputTime;
  }

  /** Set cached combo attack input time */
  void SetCachedComboAttackInputTime(float Time) {
    CachedComboAttackInputTime = Time;
  }

  /** Get cached charged attack input time */
  float GetCachedChargedAttackInputTime() const {
    return CachedChargedAttackInputTime;
  }

  /** Set cached charged attack input time */
  void SetCachedChargedAttackInputTime(float Time) {
    CachedChargedAttackInputTime = Time;
  }

protected:
  /** Called for movement input */
  void Move(const FInputActionValue &Value);

  /** Called for looking input */
  void Look(const FInputActionValue &Value);

  /** Called for combo attack input */
  void ComboAttackPressed();

  /** Called for combo attack input pressed */
  void ChargedAttackPressed();

  /** Called for combo attack input released */
  void ChargedAttackReleased();

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

  /** Handles combo attack pressed from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoComboAttackStart();

  /** Handles combo attack released from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoComboAttackEnd();

  /** Handles charged attack pressed from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoChargedAttackStart();

  /** Handles charged attack released from either controls or UI interfaces */
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoChargedAttackEnd();

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

  /** Tick function */
  virtual void Tick(float DeltaTime) override;

  /** Lock onto a target */
  void LockOntoTarget();

  /** Unlock the current target */
  void UnlockTarget();

  /** Check if has a locked target */
  bool HasLockedTarget() const;

  /** Clear the attack buffer */
  void ClearAttackBuffer();

  /** Execute buffered attack if available */
  void ExecuteBufferedAttack();

  /** Check if there's a buffered attack */
  bool HasBufferedAttack() const { return bAttackBuffered; }

  /** Get the type of buffered attack */
  int32 GetBufferedAttackType() const { return BufferedAttackType; }

  /** Get stored velocity for momentum preservation */
  const FVector &GetStoredVelocity() const { return StoredVelocity; }

  /** Get attack momentum multiplier */
  float GetAttackMomentumMultiplier() const { return AttackMomentumMultiplier; }
};
