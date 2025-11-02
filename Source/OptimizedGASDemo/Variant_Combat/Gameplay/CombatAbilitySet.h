// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySet.h"
#include "CombatAbilitySet.generated.h"

/**
 * Ability set for combat system
 */
UCLASS()
class UCombatAbilitySet : public UGameplayAbilitySet {
  GENERATED_BODY()

public:
  UCombatAbilitySet();
};
