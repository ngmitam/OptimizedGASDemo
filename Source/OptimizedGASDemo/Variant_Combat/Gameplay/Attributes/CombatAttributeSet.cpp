// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatAttributeSet.h"
#include "Net/UnrealNetwork.h"

UCombatAttributeSet::UCombatAttributeSet() {
  // Default values
  InitHealth(100.0f);
  InitMaxHealth(100.0f);
  InitDamage(10.0f);
  InitKnockbackImpulse(250.0f);
  InitLaunchImpulse(300.0f);
  InitTraceDistance(75.0f);
  InitTraceRadius(75.0f);
}

void UCombatAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, Health, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, MaxHealth, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, Damage, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, KnockbackImpulse,
                                 COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, LaunchImpulse, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, TraceDistance, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, TraceRadius, COND_None,
                                 REPNOTIFY_Always);
}

void UCombatAttributeSet::OnRep_Health(
    const FGameplayAttributeData &OldHealth) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Health, OldHealth);
}

void UCombatAttributeSet::OnRep_MaxHealth(
    const FGameplayAttributeData &OldMaxHealth) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxHealth, OldMaxHealth);
}

void UCombatAttributeSet::OnRep_Damage(
    const FGameplayAttributeData &OldDamage) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Damage, OldDamage);
}

void UCombatAttributeSet::OnRep_KnockbackImpulse(
    const FGameplayAttributeData &OldKnockbackImpulse) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, KnockbackImpulse,
                              OldKnockbackImpulse);
}

void UCombatAttributeSet::OnRep_LaunchImpulse(
    const FGameplayAttributeData &OldLaunchImpulse) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, LaunchImpulse,
                              OldLaunchImpulse);
}

void UCombatAttributeSet::OnRep_TraceDistance(
    const FGameplayAttributeData &OldTraceDistance) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, TraceDistance,
                              OldTraceDistance);
}

void UCombatAttributeSet::OnRep_TraceRadius(
    const FGameplayAttributeData &OldTraceRadius) {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, TraceRadius, OldTraceRadius);
}
