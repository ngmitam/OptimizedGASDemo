// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/CombatGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatStaminaRegenerationAbility.generated.h"

class UCombatStaminaRegenerationGameplayEffect;

/**
 * Ability that handles stamina regeneration for characters.
 * - Player: Continuous regeneration over time when not attacking
 * - Enemy: No continuous regeneration, reset to 0 if insufficient stamina for
 * attack, refill to max after 10 seconds when stamina reaches 0
 */
UCLASS()
class UCombatStaminaRegenerationAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatStaminaRegenerationAbility();

  virtual void OnGiveAbility(const FGameplayAbilityActorInfo *ActorInfo,
                             const FGameplayAbilitySpec &Spec) override;

  virtual void
  ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                  const FGameplayAbilityActorInfo *ActorInfo,
                  const FGameplayAbilityActivationInfo ActivationInfo,
                  const FGameplayEventData *TriggerEventData) override;

  virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                          const FGameplayAbilityActorInfo *ActorInfo,
                          const FGameplayAbilityActivationInfo ActivationInfo,
                          bool bReplicateEndAbility,
                          bool bWasCancelled) override;

protected:
  /** Handle depletion for enemies */
  void OnDepleted();

  /** Handle restoration for enemies */
  void OnRestored();

  /** Handle stamina change for enemies */
  void OnStaminaChanged(const FOnAttributeChangeData &Data);

  /** Handle tag changes for attacking state */
  void OnTagChanged(const FGameplayTag Tag, int32 NewCount);

  /** Check attack state and manage regeneration */
  void CheckAttackState();

  /** Apply regeneration effect for players */
  void ApplyRegenerationEffect();

  /** Remove regeneration effect */
  void RemoveRegenerationEffect();

  /** Refill stamina for enemies */
  void RefillStamina();

protected:
  /** Regeneration effect class */
  UPROPERTY(EditDefaultsOnly, Category = "Stamina Regeneration")
  TSubclassOf<UGameplayEffect> RegenerationEffectClass;

  /** Regeneration rate per second for players */
  UPROPERTY(EditDefaultsOnly, Category = "Stamina Regeneration")
  float PlayerRegenerationRate = 10.0f;

  /** Delay before refilling stamina for enemies after depletion */
  UPROPERTY(EditDefaultsOnly, Category = "Stamina Regeneration")
  float EnemyRefillDelay = 10.0f;

  /** Timer handle for enemy refill */
  FTimerHandle RefillTimerHandle;

  /** Stun effect class */
  UPROPERTY(EditDefaultsOnly, Category = "Stamina Regeneration")
  TSubclassOf<UGameplayEffect> StunEffectClass;

  /** Active effect handle */
  FActiveGameplayEffectHandle ActiveEffectHandle;

  /** Stun effect handle */
  FActiveGameplayEffectHandle StunEffectHandle;

  /** Delegate handle for stamina changes */
  FDelegateHandle StaminaChangedDelegateHandle;

  /** Delegate for stamina changes */
  FOnGameplayAttributeValueChange::FDelegate OnStaminaChangedDelegate;

  /** Timer handle for attack state check */
  FTimerHandle CheckTimer;
};
