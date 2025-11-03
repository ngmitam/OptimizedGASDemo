// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatDamageGameplayEffect.generated.h"

/**
 * GameplayEffect for applying damage to health attribute
 */
UCLASS()
class UCombatDamageGameplayEffect : public UGameplayEffect {
  GENERATED_BODY()

public:
  UCombatDamageGameplayEffect();
};
