// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/CombatGameplayAbility.h"
#include "CombatNotifyEnemiesAbility.generated.h"

class ACombatCharacter;

/**
 * Ability to notify enemies of incoming attack by performing a trace sweep
 */
UCLASS()
class UCombatNotifyEnemiesAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatNotifyEnemiesAbility();

  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

  /** Returns the CombatCharacter that owns this ability */
  class ACombatCharacter *GetCombatCharacterFromActorInfo() const;
};
