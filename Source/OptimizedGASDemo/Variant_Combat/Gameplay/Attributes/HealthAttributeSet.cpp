// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "HealthAttributeSet.h"
#include "Net/UnrealNetwork.h"

UHealthAttributeSet::UHealthAttributeSet() {
  // Default values
  InitHealth(100.0f);
  InitMaxHealth(100.0f);
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_OwnerOnly,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_OwnerOnly,
                                 REPNOTIFY_Always);
}

void UHealthAttributeSet::OnRep_Health(
    const FGameplayAttributeData &OldHealth) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldHealth);
}

void UHealthAttributeSet::OnRep_MaxHealth(
    const FGameplayAttributeData &OldMaxHealth) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldMaxHealth);
}
