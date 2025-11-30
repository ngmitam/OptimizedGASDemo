// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameplayEffect.h"
#include "CombatStaminaStunGameplayEffect.generated.h"

/**
 * GameplayEffect for stamina stun when stamina is depleted
 */
UCLASS()
class UCombatStaminaStunGameplayEffect : public UCombatGameplayEffect {
  GENERATED_BODY()

public:
  UCombatStaminaStunGameplayEffect();
};
