// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "StaminaAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"

UStaminaAttributeSet::UStaminaAttributeSet() {
  // Default values
  InitStamina(100.0f);
  InitMaxStamina(100.0f);
  InitStaminaUsed(0.0f);
}

void UStaminaAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UStaminaAttributeSet, Stamina, COND_OwnerOnly,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UStaminaAttributeSet, MaxStamina,
                                 COND_OwnerOnly, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UStaminaAttributeSet, StaminaUsed,
                                 COND_OwnerOnly, REPNOTIFY_Always);
}

void UStaminaAttributeSet::OnRep_Stamina(
    const FGameplayAttributeData &OldStamina) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UStaminaAttributeSet, Stamina, OldStamina);
}

void UStaminaAttributeSet::OnRep_MaxStamina(
    const FGameplayAttributeData &OldMaxStamina) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UStaminaAttributeSet, MaxStamina, OldMaxStamina);
}

void UStaminaAttributeSet::OnRep_StaminaUsed(
    const FGameplayAttributeData &OldStaminaUsed) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UStaminaAttributeSet, StaminaUsed,
                              OldStaminaUsed);
}

void UStaminaAttributeSet::PreAttributeChange(
    const FGameplayAttribute &Attribute, float &NewValue) {
  if (Attribute == GetStaminaAttribute()) {
    NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
  }

  Super::PreAttributeChange(Attribute, NewValue);
}
