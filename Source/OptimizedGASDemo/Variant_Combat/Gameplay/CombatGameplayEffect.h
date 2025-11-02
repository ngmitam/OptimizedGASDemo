// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatGameplayEffect.generated.h"

/**
 * Base GameplayEffect class for combat system
 */
UCLASS()
class UCombatGameplayEffect : public UGameplayEffect {
  GENERATED_BODY()

public:
  UCombatGameplayEffect();
};
