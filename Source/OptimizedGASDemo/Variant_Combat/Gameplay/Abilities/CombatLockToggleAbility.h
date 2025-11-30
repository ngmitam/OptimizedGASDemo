// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/CombatGameplayAbility.h"
#include "CombatCharacter.h"
#include "LockSystem/CombatLockSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "CombatLockToggleAbility.generated.h"

/**
 * Ability to toggle lock on/off
 */
UCLASS()
class UCombatLockToggleAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatLockToggleAbility();

protected:
  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

  /** Server-side lock state toggle with validation */
  void Server_ToggleLockState(ACombatCharacter *CombatChar);

  /** Check if the current locked target is still valid */
  bool IsTargetStillValid(ACombatCharacter *CombatChar,
                          UCombatLockSystemComponent *LockSystem) const;

  /** Draw debug visualization for lock targeting */
  void DrawDebugTrace(const FVector &StartLocation, const FVector &EndLocation,
                      const FCollisionShape &SphereShape,
                      const TArray<FHitResult> &HitResults, AActor *BestTarget,
                      UCombatLockSystemComponent *LockSystem);

private:
  /** Time of last toggle to prevent rapid toggling */
  float LastToggleTime;
};
