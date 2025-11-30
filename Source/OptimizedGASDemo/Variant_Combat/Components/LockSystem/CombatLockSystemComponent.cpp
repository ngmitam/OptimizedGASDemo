// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatLockSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "CombatCharacter.h"

UCombatLockSystemComponent::UCombatLockSystemComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  bIsLocked = false;
}

void UCombatLockSystemComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UCombatLockSystemComponent, LockedTarget);
  DOREPLIFETIME(UCombatLockSystemComponent, bIsLocked);
}

void UCombatLockSystemComponent::BeginPlay() { Super::BeginPlay(); }

void UCombatLockSystemComponent::OnRep_LockedTarget() {
  // Client syncs visual feedback when locked target changes
  if (LockedTarget && LockDecalMaterial && GetOwnerRole() != ROLE_Authority) {
    // Recreate decal on client
    if (LockDecal) {
      LockDecal->DestroyComponent();
    }
    LockDecal = UGameplayStatics::SpawnDecalAttached(
        LockDecalMaterial, FVector(100.0f, 100.0f, 100.0f),
        LockedTarget->GetRootComponent(), NAME_None, FVector(0, 0, 0),
        FRotator(0, 0, 0), EAttachLocation::SnapToTarget, 0.0f);
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
