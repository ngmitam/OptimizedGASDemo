// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayEffect.h"
#include "CombatDamageGameplayEffect.generated.h"

/**
 * GameplayEffect for applying damage to health attribute
 */
UCLASS()
class UCombatDamageGameplayEffect : public UCombatGameplayEffect {
  GENERATED_BODY()

public:
  UCombatDamageGameplayEffect();
};
