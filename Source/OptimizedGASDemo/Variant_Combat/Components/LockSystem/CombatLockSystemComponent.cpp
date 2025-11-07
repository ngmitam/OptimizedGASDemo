// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

UCombatLockSystemComponent::UCombatLockSystemComponent() {
  PrimaryComponentTick.bCanEverTick = true;
}

void UCombatLockSystemComponent::BeginPlay() { Super::BeginPlay(); }

void UCombatLockSystemComponent::LockOntoTarget() {
  AActor *Owner = GetOwner();
  if (!Owner) {
    return;
  }

  UWorld *World = GetWorld();
  if (!World) {
    return;
  }

  // Use sphere trace to find potential targets within range
  TArray<FHitResult> HitResults;
  FVector StartLocation = Owner->GetActorLocation();
  FVector EndLocation =
      StartLocation + Owner->GetActorForwardVector() * MaxLockDistance;

  // Sphere trace parameters
  FCollisionShape SphereShape =
      FCollisionShape::MakeSphere(50.0f); // Small sphere for detection
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(Owner); // Ignore self
  QueryParams.bTraceComplex = false;

  // Perform sphere trace
  bool bHit = World->SweepMultiByChannel(HitResults, StartLocation, EndLocation,
                                         FQuat::Identity, ECC_Pawn, SphereShape,
                                         QueryParams);

  AActor *BestTarget = nullptr;
  float BestScore = 0.0f; // Lower score is better (distance-based)

  // Evaluate hit results to find the best valid target (closest within cone)
  for (const FHitResult &Hit : HitResults) {
    AActor *HitActor = Hit.GetActor();
    if (!HitActor || HitActor == Owner) {
      continue;
    }

    // Check if actor is CombatEnemy
    ACombatEnemy *Enemy = Cast<ACombatEnemy>(HitActor);
    if (!Enemy) {
      continue;
    }

    // Check if has lockable tag
    UAbilitySystemComponent *ASC = Enemy->GetAbilitySystemComponent();
    if (!ASC || !ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(
                    FName("State.Lockable")))) {
      continue;
    }

    // Check if alive
    if (Enemy->GetCurrentHP() <= 0) {
      continue;
    }

    // Check if within cone angle
    FVector DirectionToActor =
        (HitActor->GetActorLocation() - StartLocation).GetSafeNormal();
    FVector OwnerForward = Owner->GetActorForwardVector();
    float Angle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(OwnerForward, DirectionToActor)));

    if (Angle <= LockConeAngle) {
      // Calculate score based on distance (closer is better)
      float Distance =
          FVector::Dist(StartLocation, HitActor->GetActorLocation());
      float Score = Distance; // Could be modified to include other factors like
                              // threat level

      if (!BestTarget || Score < BestScore) {
        BestTarget = HitActor;
        BestScore = Score;
      }
    }
  }

  // Debug drawing
  DrawDebugTrace(StartLocation, EndLocation, SphereShape, HitResults,
                 BestTarget);

  for (const FHitResult &Hit : HitResults) {
    AActor *HitActor = Hit.GetActor();
    if (!HitActor || HitActor == Owner) {
      continue;
    }

    // Check if actor is CombatEnemy
    ACombatEnemy *Enemy = Cast<ACombatEnemy>(HitActor);
    if (!Enemy) {
      continue;
    }

    // Check if has lockable tag
    UAbilitySystemComponent *ASC = Enemy->GetAbilitySystemComponent();
    if (!ASC || !ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(
                    FName("State.Lockable")))) {
      continue;
    }

    // Check if alive
    if (Enemy->GetCurrentHP() <= 0) {
      continue;
    }

    // Check if within cone angle
    FVector DirectionToActor =
        (HitActor->GetActorLocation() - StartLocation).GetSafeNormal();
    FVector OwnerForward = Owner->GetActorForwardVector();
    float Angle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(OwnerForward, DirectionToActor)));

    if (Angle <= LockConeAngle) {
      // Calculate score based on distance (closer is better)
      float Distance =
          FVector::Dist(StartLocation, HitActor->GetActorLocation());
      float Score = Distance; // Could be modified to include other factors like
                              // threat level

      if (!BestTarget || Score < BestScore) {
        BestTarget = HitActor;
        BestScore = Score;
      }
    }
  }

  LockedTarget = BestTarget;

  // Spawn decal for visual feedback
  if (LockedTarget && LockDecalMaterial) {
    LockDecal = UGameplayStatics::SpawnDecalAttached(
        LockDecalMaterial, FVector(100.0f, 100.0f, 100.0f),
        LockedTarget->GetRootComponent(), NAME_None, FVector(0, 0, 0),
        FRotator(0, 0, 0), EAttachLocation::SnapToTarget, 0.0f);
  }
}

void UCombatLockSystemComponent::UnlockTarget() {
  if (LockDecal) {
    LockDecal->DestroyComponent();
    LockDecal = nullptr;
  }

  LockedTarget = nullptr;
}

bool UCombatLockSystemComponent::IsTargetStillValid() const {
  if (!LockedTarget) {
    return false;
  }

  AActor *Owner = GetOwner();
  if (!Owner) {
    return false;
  }

  // Check if target is still a valid CombatEnemy with lockable tag
  ACombatEnemy *Enemy = Cast<ACombatEnemy>(LockedTarget);
  if (!Enemy) {
    return false;
  }

  UAbilitySystemComponent *ASC = Enemy->GetAbilitySystemComponent();
  if (!ASC || !ASC->HasMatchingGameplayTag(
                  FGameplayTag::RequestGameplayTag(FName("State.Lockable")))) {
    return false;
  }

  // Check if target is alive
  if (Enemy->GetCurrentHP() <= 0) {
    return false;
  }

  // Check distance
  FVector OwnerLocation = Owner->GetActorLocation();
  FVector TargetLocation = LockedTarget->GetActorLocation();
  float Distance = FVector::Dist(OwnerLocation, TargetLocation);
  if (Distance > MaxLockDistance) {
    return false;
  }

  // Check angle
  FVector OwnerForward = Owner->GetActorForwardVector();
  FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
  float Angle = FMath::RadiansToDegrees(
      FMath::Acos(FVector::DotProduct(OwnerForward, DirectionToTarget)));
  if (Angle > LockConeAngle) {
    return false;
  }

  return true;
}

void UCombatLockSystemComponent::DrawDebugTrace(
    const FVector &StartLocation, const FVector &EndLocation,
    const FCollisionShape &SphereShape, const TArray<FHitResult> &HitResults,
    AActor *BestTarget) {
  UWorld *World = GetWorld();
  if (!World)
    return;

  // Draw the sphere trace path
  DrawDebugSphere(World, StartLocation, SphereShape.GetSphereRadius(), 12,
                  FColor::Blue, false, 2.0f);
  DrawDebugSphere(World, EndLocation, SphereShape.GetSphereRadius(), 12,
                  FColor::Blue, false, 2.0f);
  DrawDebugLine(World, StartLocation, EndLocation, FColor::Blue, false, 2.0f, 0,
                2.0f);

  // Draw cone angle visualization
  AActor *Owner = GetOwner();
  if (Owner) {
    FVector OwnerForward = Owner->GetActorForwardVector();
    FVector ConeLeft =
        OwnerForward.RotateAngleAxis(-LockConeAngle, FVector::UpVector);
    FVector ConeRight =
        OwnerForward.RotateAngleAxis(LockConeAngle, FVector::UpVector);

    FVector ConeEndLeft = StartLocation + ConeLeft * MaxLockDistance;
    FVector ConeEndRight = StartLocation + ConeRight * MaxLockDistance;

    DrawDebugLine(World, StartLocation, ConeEndLeft, FColor::Yellow, false,
                  2.0f, 0, 1.0f);
    DrawDebugLine(World, StartLocation, ConeEndRight, FColor::Yellow, false,
                  2.0f, 0, 1.0f);
    DrawDebugLine(World, ConeEndLeft, ConeEndRight, FColor::Yellow, false, 2.0f,
                  0, 1.0f);
  }

  // Draw hit results
  for (const FHitResult &Hit : HitResults) {
    AActor *HitActor = Hit.GetActor();
    if (HitActor) {
      FColor HitColor = FColor::Red; // Default red for invalid targets

      // Check if this is a valid CombatEnemy
      ACombatEnemy *Enemy = Cast<ACombatEnemy>(HitActor);
      if (Enemy) {
        UAbilitySystemComponent *ASC = Enemy->GetAbilitySystemComponent();
        if (ASC &&
            ASC->HasMatchingGameplayTag(
                FGameplayTag::RequestGameplayTag(FName("State.Lockable"))) &&
            Enemy->GetCurrentHP() > 0) {
          HitColor = FColor::Green; // Valid target
        } else {
          HitColor = FColor::Orange; // Enemy but not lockable/alive
        }
      }

      // Highlight the best target
      if (HitActor == BestTarget) {
        HitColor = FColor::Cyan;
        DrawDebugSphere(World, HitActor->GetActorLocation(), 50.0f, 8, HitColor,
                        false, 2.0f);
      }

      DrawDebugPoint(World, Hit.ImpactPoint, 10.0f, HitColor, false, 2.0f);
      DrawDebugString(World, Hit.ImpactPoint + FVector(0, 0, 50),
                      HitActor->GetName(), nullptr, HitColor, 2.0f);
    }
  }
}
