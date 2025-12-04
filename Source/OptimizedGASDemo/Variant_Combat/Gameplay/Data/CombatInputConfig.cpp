// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatInputConfig.h"

UCombatInputConfig::UCombatInputConfig() {}

const FCombatInputMapping *UCombatInputConfig::FindMappingForInputAction(
    const UInputAction *InputAction) const {
  for (const FCombatInputMapping &Mapping : InputMappings) {
    if (Mapping.InputAction == InputAction) {
      return &Mapping;
    }
  }
  return nullptr;
}

void UCombatInputConfig::GetAllInputActions(
    TArray<UInputAction *> &OutInputActions) const {
  for (const FCombatInputMapping &Mapping : InputMappings) {
    if (Mapping.InputAction) {
      OutInputActions.Add(Mapping.InputAction);
    }
  }
}
