// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "Variant_Combat/CombatGameMode.h"
#include "Variant_Combat/CombatPlayerState.h"

ACombatGameMode::ACombatGameMode() {
  PlayerStateClass = ACombatPlayerState::StaticClass();
}