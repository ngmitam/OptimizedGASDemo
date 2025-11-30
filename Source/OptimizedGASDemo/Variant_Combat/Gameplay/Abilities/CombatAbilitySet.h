// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "CombatAbilitySet.generated.h"

class UAttributeSet;
class UGameplayEffect;

/** Handle for tracking granted abilities, effects, and attributes */
USTRUCT(BlueprintType)
struct FCombatAbilitySetHandle {
  GENERATED_BODY()

  void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle &Handle);
  void AddGameplayEffectHandle(const FActiveGameplayEffectHandle &Handle);
  void AddAttributeSet(UAttributeSet *AttributeSet);
  void TakeFromAbilitySystem(UAbilitySystemComponent *ASC);
  bool IsValid() const;
  void Reset();

private:
  TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
  TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
  TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/** Application info for attribute sets */
USTRUCT(BlueprintType)
struct FGameplayAttributeApplicationInfo {
  GENERATED_BODY()

  /** Attribute set to grant */
  UPROPERTY(EditAnywhere, Category = AbilitySet)
  TSubclassOf<UAttributeSet> AttributeSet;

  /** Activation policy for this attribute set */
  UPROPERTY(EditAnywhere, Category = AbilitySet)
  EGameplayEffectGrantedAbilityRemovePolicy RemovalPolicy =
      EGameplayEffectGrantedAbilityRemovePolicy::CancelAbilityImmediately;
};

/** Application info for gameplay effects */
USTRUCT(BlueprintType)
struct FGameplayEffectApplicationInfo {
  GENERATED_BODY()

  /** Gameplay effect to grant */
  UPROPERTY(EditAnywhere, Category = AbilitySet)
  TSubclassOf<UGameplayEffect> GameplayEffect;

  /** Application policy for this effect */
  UPROPERTY(EditAnywhere, Category = AbilitySet)
  EGameplayEffectGrantedAbilityRemovePolicy RemovalPolicy =
      EGameplayEffectGrantedAbilityRemovePolicy::CancelAbilityImmediately;
};

/**
 * Ability set for combat system
 */
UCLASS()
class UCombatAbilitySet : public UGameplayAbilitySet {
  GENERATED_BODY()

public:
  UCombatAbilitySet();

  /** Grants the ability set to the specified ability system component */
  void GiveToAbilitySystem(UAbilitySystemComponent *ASC,
                           FCombatAbilitySetHandle &OutAbilitySetHandle,
                           UObject *SourceObject = nullptr) const;

  /** Takes the ability set from the specified ability system component */
  void TakeFromAbilitySystem(UAbilitySystemComponent *ASC,
                             FCombatAbilitySetHandle &AbilitySetHandle);

  /** Granted gameplay effects */
  UPROPERTY(EditAnywhere, Category = AbilitySet)
  TArray<FGameplayEffectApplicationInfo> GrantedGameplayEffects;

  /** Granted attribute sets */
  UPROPERTY(EditAnywhere, Category = AbilitySet)
  TArray<FGameplayAttributeApplicationInfo> GrantedAttributes;
};
