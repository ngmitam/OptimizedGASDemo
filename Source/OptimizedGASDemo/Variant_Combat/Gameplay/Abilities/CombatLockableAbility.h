// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CombatLockableAbility.generated.h"

/**
 * Ability that grants the lockable tag to make the enemy lockable
 */
UCLASS()
class OPTIMIZEDGASDEMO_API UCombatLockableAbility : public UGameplayAbility {
  GENERATED_BODY()

public:
  UCombatLockableAbility();

  virtual void OnGiveAbility(const FGameplayAbilityActorInfo *ActorInfo,
                             const FGameplayAbilitySpec &Spec) override;
};
