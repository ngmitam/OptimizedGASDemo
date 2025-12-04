// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "CombatCharacter.h"

UCombatLockSystemComponent::UCombatLockSystemComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  bIsLocked = false;
}

void UCombatLockSystemComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UCombatLockSystemComponent, LockedTarget);
  DOREPLIFETIME(UCombatLockSystemComponent, bIsLocked);
}

void UCombatLockSystemComponent::BeginPlay() {
  Super::BeginPlay();
  StateDeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));

  // Set up timer to check locked target validity periodically
  GetWorld()->GetTimerManager().SetTimer(
      CheckTimerHandle, this,
      &UCombatLockSystemComponent::CheckLockedTargetValidity, CheckInterval,
      true);
}

void UCombatLockSystemComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  // Clear the timer
  if (CheckTimerHandle.IsValid()) {
    GetWorld()->GetTimerManager().ClearTimer(CheckTimerHandle);
  }
}

void UCombatLockSystemComponent::CheckLockedTargetValidity() {
  // First check if owner is dead - if so, unlock immediately
  AActor *Owner = GetOwner();
  if (Owner) {
    UAbilitySystemComponent *OwnerASC =
        UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
    if (OwnerASC && OwnerASC->HasMatchingGameplayTag(StateDeadTag)) {
      // Owner is dead, unlock and clean up decal
      if (LockDecal) {
        LockDecal->DestroyComponent();
        LockDecal = nullptr;
      }
      SetLockedTarget(nullptr);
      SetIsLocked(false);
      return;
    }
  }

  if (!LockedTarget || !LockedTarget->IsValidLowLevel()) {
    // Target doesn't exist anymore
    SetLockedTarget(nullptr);
    SetIsLocked(false);
  } else {
    // Check if target is dead
    UAbilitySystemComponent *ASC =
        UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(LockedTarget);
    if (ASC && ASC->HasMatchingGameplayTag(StateDeadTag)) {
      // Target is dead, unlock
      SetLockedTarget(nullptr);
      SetIsLocked(false);
    }
  }
}

void UCombatLockSystemComponent::OnRep_LockedTarget() {
  // Client syncs visual feedback when locked target changes
  if (LockedTarget && LockDecalMaterial) {
    // Recreate decal on client
    if (LockDecal) {
      LockDecal->DestroyComponent();
    }

    // Calculate decal size based on target's capsule
    FVector DecalSize = FVector(100.0f, 100.0f, 100.0f); // Default
    if (ACombatBase *CombatBase = Cast<ACombatBase>(LockedTarget)) {
      if (UCapsuleComponent *Capsule = CombatBase->GetCapsuleComponent()) {
        float Radius = Capsule->GetScaledCapsuleRadius();
        float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        // Set decal size to fit the capsule base (circle under feet)
        DecalSize = FVector(Radius, Radius, HalfHeight);
      }
    }

    LockDecal = UGameplayStatics::SpawnDecalAttached(
        LockDecalMaterial, DecalSize, LockedTarget->GetRootComponent(),
        NAME_None, FVector(0, 0, 0), FRotator(0, 0, 0),
        EAttachLocation::SnapToTarget, 0.0f);
  } else if (!LockedTarget && LockDecal) {
    LockDecal->DestroyComponent();
    LockDecal = nullptr;
  }
}

void UCombatLockSystemComponent::OnRep_IsLocked() {
  // Client syncs gameplay tags and camera control when lock state changes
  AActor *Owner = GetOwner();
  if (!Owner)
    return;

  UAbilitySystemComponent *ASC = nullptr;
  // Try to get ASC from CombatCharacter
  ACombatCharacter *CombatChar = Cast<ACombatCharacter>(Owner);
  if (CombatChar) {
    ASC = CombatChar->GetAbilitySystemComponent();
  }

  if (bIsLocked) {
    // Add locked state tag
    if (ASC) {
      ASC->AddLooseGameplayTag(
          FGameplayTag::RequestGameplayTag(FName("State.Locked")));
    }
    // Disable pawn control rotation for camera lock
    if (CombatChar && CombatChar->GetCameraBoom()) {
      CombatChar->GetCameraBoom()->bUsePawnControlRotation = false;
    }
  } else {
    // Remove locked state tag
    if (ASC) {
      ASC->RemoveLooseGameplayTag(
          FGameplayTag::RequestGameplayTag(FName("State.Locked")));
    }
    // Re-enable pawn control rotation
    if (CombatChar && CombatChar->GetCameraBoom()) {
      CombatChar->GetCameraBoom()->bUsePawnControlRotation = true;
    }
  }
}

void UCombatLockSystemComponent::Client_PredictToggleLock() {
  // Client prediction: immediately toggle for responsive feel
  if (HasLockedTarget()) {
    // Unlock logic
    if (LockDecal) {
      LockDecal->DestroyComponent();
      LockDecal = nullptr;
    }
    LockedTarget = nullptr;
    bIsLocked = false;
  } else {
    // Lock logic - simplified for client prediction, full logic on server
    // For prediction, just assume lock onto something if possible
    // But to keep simple, perhaps do nothing or minimal
    // Since server will reconcile, client can just toggle state
    bIsLocked = !bIsLocked;
  }

  // Server will reconcile via replication
}
