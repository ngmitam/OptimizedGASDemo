// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "DamageAttributeSet.h"
#include "Net/UnrealNetwork.h"

UDamageAttributeSet::UDamageAttributeSet() {
  // Default values
  InitDamage(10.0f);
  InitKnockbackImpulse(250.0f);
  InitLaunchImpulse(300.0f);
}

void UDamageAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributeSet, Damage, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributeSet, KnockbackImpulse,
                                 COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributeSet, LaunchImpulse, COND_None,
                                 REPNOTIFY_Always);
}

void UDamageAttributeSet::OnRep_Damage(
    const FGameplayAttributeData &OldDamage) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributeSet, Damage, OldDamage);
}

void UDamageAttributeSet::OnRep_KnockbackImpulse(
    const FGameplayAttributeData &OldKnockbackImpulse) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributeSet, KnockbackImpulse,
                              OldKnockbackImpulse);
}

void UDamageAttributeSet::OnRep_LaunchImpulse(
    const FGameplayAttributeData &OldLaunchImpulse) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributeSet, LaunchImpulse,
                              OldLaunchImpulse);
}
