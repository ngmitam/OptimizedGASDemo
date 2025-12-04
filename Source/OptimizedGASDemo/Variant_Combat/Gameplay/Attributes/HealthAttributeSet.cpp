// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "HealthAttributeSet.h"
#include "Net/UnrealNetwork.h"

UHealthAttributeSet::UHealthAttributeSet() {
  // Default values
  InitHealth(1000.0f);
  InitMaxHealth(1000.0f);
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

void UHealthAttributeSet::PreAttributeChange(
    const FGameplayAttribute &Attribute, float &NewValue) {
  Super::PreAttributeChange(Attribute, NewValue);

  if (Attribute == GetHealthAttribute()) {
    // Clamp health to not exceed max health
    NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
  }
}

void UHealthAttributeSet::PostAttributeChange(
    const FGameplayAttribute &Attribute, float OldValue, float NewValue) {
  Super::PostAttributeChange(Attribute, OldValue, NewValue);
}
