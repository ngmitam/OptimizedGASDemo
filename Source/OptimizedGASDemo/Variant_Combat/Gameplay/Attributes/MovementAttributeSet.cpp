// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "MovementAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMovementAttributeSet::UMovementAttributeSet() {
  MovementSpeed.SetBaseValue(600.0f);
  MovementSpeed.SetCurrentValue(600.0f);
  MaxMovementSpeed.SetBaseValue(600.0f);
  MaxMovementSpeed.SetCurrentValue(600.0f);
}

void UMovementAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, MovementSpeed,
                                 COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, MaxMovementSpeed,
                                 COND_None, REPNOTIFY_Always);
}

void UMovementAttributeSet::OnRep_MovementSpeed(
    const FGameplayAttributeData &OldMovementSpeed) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, MovementSpeed,
                              OldMovementSpeed);
}

void UMovementAttributeSet::OnRep_MaxMovementSpeed(
    const FGameplayAttributeData &OldMaxMovementSpeed) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, MaxMovementSpeed,
                              OldMaxMovementSpeed);
}
