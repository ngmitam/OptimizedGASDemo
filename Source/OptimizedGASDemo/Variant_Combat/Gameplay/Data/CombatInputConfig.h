// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagsManager.h"
#include "InputAction.h"
#include "GameplayAbilitySpec.h"
#include "CombatInputConfig.generated.h"

class UInputAction;

/**
 * Trigger data for an input action
 */
USTRUCT(BlueprintType)
struct FCombatAbilityTriggerData {
  GENERATED_BODY()

  /** The trigger tag for this ability */
  UPROPERTY(EditDefaultsOnly, Category = "Trigger")
  FGameplayTag TriggerTag;

  /** The trigger event to listen for */
  UPROPERTY(EditDefaultsOnly, Category = "Trigger")
  ETriggerEvent TriggerEvent = ETriggerEvent::Started;

  /** Whether this trigger should be consumed when processed */
  UPROPERTY(EditDefaultsOnly, Category = "Trigger")
  bool bTriggerOnce = false;
};

/**
 * Input mapping for abilities
 */
USTRUCT(BlueprintType)
struct FCombatInputMapping {
  GENERATED_BODY()

  /** The input action */
  UPROPERTY(EditDefaultsOnly, Category = "Input")
  TObjectPtr<UInputAction> InputAction;

  /** The ability triggers for this input action */
  UPROPERTY(EditDefaultsOnly, Category = "Input")
  TArray<FCombatAbilityTriggerData> AbilityTriggers;

  /** Whether this input should be consumed when processed */
  UPROPERTY(EditDefaultsOnly, Category = "Input")
  bool bConsumeInput = true;
};

/**
 * Input configuration data asset
 */
UCLASS()
class UCombatInputConfig : public UDataAsset {
  GENERATED_BODY()

public:
  UCombatInputConfig();

  /** Input mappings for abilities */
  UPROPERTY(EditDefaultsOnly, Category = "Input")
  TArray<FCombatInputMapping> InputMappings;

  /** Find input mapping by input action */
  const FCombatInputMapping *
  FindMappingForInputAction(const UInputAction *InputAction) const;

  /** Get all input actions */
  void GetAllInputActions(TArray<UInputAction *> &OutInputActions) const;
};
