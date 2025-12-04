// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/CombatGameplayAbility.h"
#include "GameplayEffect.h"
#include "CombatComboAttackAbility.generated.h"

class UAnimMontage;

/**
 * Ability for performing combo attacks using GAS
 */
UCLASS()
class UCombatComboAttackAbility : public UCombatGameplayAbility {
  GENERATED_BODY()

public:
  UCombatComboAttackAbility();

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

  /** Handle combo next event */
  void HandleComboNext(const FGameplayEventData *EventData);

  /** Handle combo start event for input caching */
  void HandleComboStart(const FGameplayEventData *EventData);

  /** Called when montage ends */
  void OnMontageEnded(UAnimMontage *Montage, bool bInterrupted);

  /** Stamina costs for each combo section */
  UPROPERTY(EditAnywhere, Category = "Combo")
  TArray<float> StaminaCosts = {10.0f, 15.0f, 20.0f};

  /** Effect class to use for stamina cost */
  UPROPERTY(EditAnywhere, Category = "Combo")
  TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

  /** Effect class to use for setting stamina used */
  UPROPERTY(EditAnywhere, Category = "Combo")
  TSubclassOf<UGameplayEffect> StaminaUsedEffectClass;

  /** Time tolerance for combo input caching */
  UPROPERTY(EditAnywhere, Category = "Combo")
  float ComboInputCacheTimeTolerance = 1.0f;

protected:
  /** Current combo count */
  int32 CurrentComboCount = 0;

  /** Target combo count for AI */
  int32 TargetComboCount = 0;

  /** Combo attack montage */
  UAnimMontage *ComboAttackMontage = nullptr;

  /** Combo section names */
  TArray<FName> ComboSectionNames;

  /** Montage ended delegate */
  FOnMontageEnded MontageEndedDelegate;

  /** Cached combo input time */
  float CachedComboInputTime = 0.0f;

  /** Whether the combo has started */
  bool bComboStarted = false;
};
