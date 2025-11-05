// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagsManager.h"
#include "Components/ActorComponent.h"
#include "Components/DecalComponent.h"
#include "AI/CombatEnemy.h"
#include "CombatLockSystemComponent.generated.h"

/**
 * Component that handles locking onto targets in combat
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UCombatLockSystemComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UCombatLockSystemComponent();

  /** Find and lock onto the nearest target */
  void LockOntoTarget();

  /** Unlock from current target */
  void UnlockTarget();

  /** Get the currently locked target */
  AActor *GetLockedTarget() const { return LockedTarget; }

  /** Check if we have a locked target */
  bool HasLockedTarget() const { return LockedTarget != nullptr; }

  /** Check if the current target is still valid */
  bool IsTargetStillValid() const;

  /** Draw debug trace visualization */
  void DrawDebugTrace(const FVector &StartLocation, const FVector &EndLocation,
                      const FCollisionShape &SphereShape,
                      const TArray<FHitResult> &HitResults, AActor *BestTarget);

protected:
  virtual void BeginPlay() override;

  /** Draw debug information */
  void DrawDebugInfo();

  /** The currently locked target */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock System")
  AActor *LockedTarget;

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
