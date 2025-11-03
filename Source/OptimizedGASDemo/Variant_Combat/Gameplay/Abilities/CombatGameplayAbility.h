// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CombatGameplayAbility.generated.h"

class ACombatCharacter;
class ACombatEnemy;

/**
 * Base GameplayAbility class for combat system
 */
UCLASS()
class UCombatGameplayAbility : public UGameplayAbility {
  GENERATED_BODY()

public:
  UCombatGameplayAbility();

  //~UGameplayAbility interface
  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;
  virtual bool CanActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo *ActorInfo,
      const FGameplayTagContainer *SourceTags = nullptr,
      const FGameplayTagContainer *TargetTags = nullptr,
      OUT FGameplayTagContainer *OptionalRelevantTags = nullptr) const override;
  virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                          const FGameplayAbilityActorInfo *ActorInfo,
                          const FGameplayAbilityActivationInfo ActivationInfo,
                          bool bReplicateEndAbility,
                          bool bWasCancelled) override;
  //~End of UGameplayAbility interface

protected:
  /** Called when ability is activated */
  UFUNCTION(BlueprintImplementableEvent, Category = Ability)
  void OnAbilityActivated();

  /** Called when ability ends */
  UFUNCTION(BlueprintImplementableEvent, Category = Ability)
  void OnAbilityEnded(bool bWasCancelled);

  /** Returns the CombatCharacter that owns this ability */
  class ACombatCharacter *GetCombatCharacterFromActorInfo() const;

  /** Returns the CombatEnemy that owns this ability */
  class ACombatEnemy *GetCombatEnemyFromActorInfo() const;
};
