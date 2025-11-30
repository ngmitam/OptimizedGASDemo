// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagsManager.h"
#include "Components/ActorComponent.h"
#include "Components/DecalComponent.h"
#include "AI/CombatEnemy.h"
#include "CombatLockSystemComponent.generated.h"

// Forward declarations
class ACombatCharacter;

/**
 * Component that handles locking onto targets in combat
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UCombatLockSystemComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UCombatLockSystemComponent();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  /** Get the currently locked target */
  AActor *GetLockedTarget() const { return LockedTarget; }

  /** Check if we have a locked target */
  bool HasLockedTarget() const { return LockedTarget != nullptr; }

  /** Set the locked target (for ability use) */
  void SetLockedTarget(AActor *NewTarget) {
    LockedTarget = NewTarget;
    bIsLocked = (LockedTarget != nullptr);
  }

  /** Set lock state (for ability use) */
  void SetIsLocked(bool bLocked) { bIsLocked = bLocked; }

  /** Get lock state */
  bool GetIsLocked() const { return bIsLocked; }

  /** Get max lock distance */
  float GetMaxLockDistance() const { return MaxLockDistance; }

  /** Get lock cone angle */
  float GetLockConeAngle() const { return LockConeAngle; }

  /** Get lock decal material */
  UMaterialInterface *GetLockDecalMaterial() const { return LockDecalMaterial; }

  /** Get lock decal */
  UDecalComponent *GetLockDecal() const { return LockDecal; }

  /** Set lock decal */
  void SetLockDecal(UDecalComponent *NewDecal) { LockDecal = NewDecal; }

  /** Client prediction for lock toggle */
  void Client_PredictToggleLock();

protected:
  /** Called when locked target changes */
  UFUNCTION()
  void OnRep_LockedTarget();

  /** Called when lock state changes */
  UFUNCTION()
  void OnRep_IsLocked();

protected:
  virtual void BeginPlay() override;

  /** The currently locked target */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock System",
            ReplicatedUsing = OnRep_LockedTarget)
  AActor *LockedTarget;

  /** Whether we currently have a locked target (for replication) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock System",
            ReplicatedUsing = OnRep_IsLocked)
  bool bIsLocked;

  /** Maximum distance to lock onto targets */
  UPROPERTY(EditAnywhere, Category = "Lock System",
            meta = (ClampMin = 0, Units = "cm"))
  float MaxLockDistance = 2000.0f;

  /** Angle in degrees for lock cone */
  UPROPERTY(EditAnywhere, Category = "Lock System",
            meta = (ClampMin = 0, ClampMax = 180))
  float LockConeAngle = 45.0f;

  /** Tags to identify valid targets */
  UPROPERTY(EditAnywhere, Category = "Lock System")
  FGameplayTagContainer ValidTargetTags;

  /** Material for the lock decal */
  UPROPERTY(EditAnywhere, Category = "Lock System")
  UMaterialInterface *LockDecalMaterial;

  /** Decal component for visual feedback */
  UDecalComponent *LockDecal;
};
