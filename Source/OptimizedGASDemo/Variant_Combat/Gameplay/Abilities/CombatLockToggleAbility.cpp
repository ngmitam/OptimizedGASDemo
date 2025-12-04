// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockToggleAbility.h"
#include "CombatCharacter.h"
#include "LockSystem/CombatLockSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "CombatEnemy.h"

UCombatLockToggleAbility::UCombatLockToggleAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
  LastToggleTime = -1.0f;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Lock.Toggle")));
  SetAssetTags(AssetTags);

  // Add trigger to activate on lock toggle event
  FAbilityTriggerData TriggerData;
  TriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Lock.Toggle"));
  TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(TriggerData);
}

void UCombatLockToggleAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  // For LocalPredicted, ensure only server activates (client prediction is
  // handled separately)
  if (!ActorInfo->AvatarActor->HasAuthority()) {
    return; // Just return, don't end ability since it didn't start
  }

  // Prevent rapid activations
  float CurrentTime = GetWorld()->GetTimeSeconds();
  if (LastToggleTime >= 0 && CurrentTime - LastToggleTime < 0.2f) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }
  LastToggleTime = CurrentTime;

  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  ACombatCharacter *CombatChar =
      Cast<ACombatCharacter>(ActorInfo->AvatarActor.Get());
  if (CombatChar && CombatChar->GetLockSystemComponent()) {
    // Server: perform the actual lock/unlock logic
    Server_ToggleLockState(CombatChar);
  }

  EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UCombatLockToggleAbility::Server_ToggleLockState(
    ACombatCharacter *CombatChar) {
  if (!CombatChar || !CombatChar->GetLockSystemComponent()) {
    return;
  }

  UCombatLockSystemComponent *LockSystem = CombatChar->GetLockSystemComponent();

  if (LockSystem->HasLockedTarget()) { // Assuming HasLockedTarget() checks
                                       // LockedTarget != nullptr
    // Validate that target is still valid before unlocking
    if (IsTargetStillValid(CombatChar, LockSystem)) {
      // Unlock logic
      if (LockSystem->GetLockDecal()) {
        LockSystem->GetLockDecal()->DestroyComponent();
        LockSystem->SetLockDecal(nullptr);
      }
      LockSystem->SetLockedTarget(nullptr);
      LockSystem->SetIsLocked(false);
    } else {
      // Target became invalid, force unlock
      if (LockSystem->GetLockDecal()) {
        LockSystem->GetLockDecal()->DestroyComponent();
        LockSystem->SetLockDecal(nullptr);
      }
      LockSystem->SetLockedTarget(nullptr);
      LockSystem->SetIsLocked(false);
    }
  } else {
    // Try to lock onto a new target - inline LockOntoTarget logic
    AActor *Owner = CombatChar;
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
    FVector EndLocation = StartLocation + Owner->GetActorForwardVector() *
                                              LockSystem->GetMaxLockDistance();

    // Sphere trace parameters
    FCollisionShape SphereShape =
        FCollisionShape::MakeSphere(50.0f); // Small sphere for detection
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner); // Ignore self
    QueryParams.bTraceComplex = false;

    // Perform sphere trace
    bool bHit = World->SweepMultiByChannel(HitResults, StartLocation,
                                           EndLocation, FQuat::Identity,
                                           ECC_Pawn, SphereShape, QueryParams);

    AActor *BestTarget = nullptr;
    float BestScore = 0.0f; // Lower score is better (distance-based)
    int32 ValidEnemyCount = 0;

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
      UAbilitySystemComponent *ASC =
          UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Enemy);
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

      if (Angle <= LockSystem->GetLockConeAngle()) {
        // Calculate score based on distance (closer is better)
        float Distance =
            FVector::Dist(StartLocation, HitActor->GetActorLocation());
        float Score = Distance; // Could be modified to include other factors
                                // like threat level

        if (!BestTarget || Score < BestScore) {
          BestTarget = HitActor;
          BestScore = Score;
        }
        ValidEnemyCount++;
      }
    }

    // Debug drawing
    DrawDebugTrace(StartLocation, EndLocation, SphereShape, HitResults,
                   BestTarget, LockSystem);

    LockSystem->SetLockedTarget(BestTarget);

    // Spawn decal for visual feedback
    if (LockSystem->GetLockedTarget() && LockSystem->GetLockDecalMaterial()) {
      // Calculate decal size based on target's capsule component
      FVector DecalSize = FVector(100.0f, 100.0f, 100.0f);
      if (UCapsuleComponent *Capsule = Cast<UCapsuleComponent>(
              LockSystem->GetLockedTarget()->GetRootComponent())) {
        float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
        float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        // Make decal slightly larger than the capsule for better visibility
        DecalSize = FVector(CapsuleRadius * 2.5f, CapsuleRadius * 2.5f,
                            CapsuleHalfHeight * 2.2f);
      }

      UDecalComponent *NewDecal = UGameplayStatics::SpawnDecalAttached(
          LockSystem->GetLockDecalMaterial(), DecalSize,
          LockSystem->GetLockedTarget()->GetRootComponent(), NAME_None,
          FVector(0, 0, 0), FRotator(0, 0, 0), EAttachLocation::SnapToTarget,
          0.0f);
      LockSystem->SetLockDecal(NewDecal);
    }
  }
}

bool UCombatLockToggleAbility::IsTargetStillValid(
    ACombatCharacter *CombatChar,
    UCombatLockSystemComponent *LockSystem) const {
  if (!LockSystem->GetLockedTarget()) {
    return false;
  }

  AActor *Owner = CombatChar;
  if (!Owner) {
    return false;
  }

  // Check if target is still a valid CombatEnemy with lockable tag
  ACombatEnemy *Enemy = Cast<ACombatEnemy>(LockSystem->GetLockedTarget());
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
  FVector TargetLocation = LockSystem->GetLockedTarget()->GetActorLocation();
  float Distance = FVector::Dist(OwnerLocation, TargetLocation);
  if (Distance > LockSystem->GetMaxLockDistance()) {
    return false;
  }

  // Note: Not checking angle to allow lock to persist even if target moves out
  // of cone

  return true;
}

void UCombatLockToggleAbility::DrawDebugTrace(
    const FVector &StartLocation, const FVector &EndLocation,
    const FCollisionShape &SphereShape, const TArray<FHitResult> &HitResults,
    AActor *BestTarget, UCombatLockSystemComponent *LockSystem) {
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
  AActor *Owner = LockSystem->GetOwner();
  if (Owner) {
    FVector OwnerForward = Owner->GetActorForwardVector();
    FVector ConeLeft = OwnerForward.RotateAngleAxis(
        -LockSystem->GetLockConeAngle(), FVector::UpVector);
    FVector ConeRight = OwnerForward.RotateAngleAxis(
        LockSystem->GetLockConeAngle(), FVector::UpVector);

    FVector ConeEndLeft =
        StartLocation + ConeLeft * LockSystem->GetMaxLockDistance();
    FVector ConeEndRight =
        StartLocation + ConeRight * LockSystem->GetMaxLockDistance();

    DrawDebugLine(World, StartLocation, ConeEndLeft, FColor::Yellow, false,
                  2.0f, 0, 1.0f);
    DrawDebugLine(World, StartLocation, ConeEndRight, FColor::Yellow, false,
                  2.0f, 0, 1.0f);
    DrawDebugLine(World, StartLocation, ConeEndRight, FColor::Yellow, false,
                  2.0f, 0, 1.0f); // Fixed: was ConeEndLeft to ConeEndRight
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
