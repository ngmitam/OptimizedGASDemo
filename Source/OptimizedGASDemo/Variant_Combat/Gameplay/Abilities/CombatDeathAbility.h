// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayAbility.h"
#include "CombatDeathAbility.generated.h"

class ACombatCharacter;
class ACombatEnemy;

/**
 * GameplayAbility for death
 */
UCLASS()
class UCombatDeathAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatDeathAbility();

  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

protected:
  /** Time before respawning/removing character */
  UPROPERTY(EditDefaultsOnly, Category = "Death", meta = (ClampMin = 0))
  float DeathDuration = 3.0f;

  /** Whether to respawn or remove character */
  UPROPERTY(EditDefaultsOnly, Category = "Death")
  bool bShouldRespawn = true;

  /** Timer for death duration */
  FTimerHandle DeathTimer;

  /** Called when death timer expires */
  void OnDeathTimerExpired();

  /** Returns the CombatCharacter that owns this ability */
  class ACombatCharacter *GetCombatCharacterFromActorInfo() const;

  /** Returns the CombatEnemy that owns this ability */
  class ACombatEnemy *GetCombatEnemyFromActorInfo() const;
};
