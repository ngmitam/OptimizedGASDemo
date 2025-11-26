// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attributes/CombatAttributeSet.h"

ACombatPlayerState::ACombatPlayerState() {
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));
  AbilitySystemComponent->SetIsReplicated(true);

  AttributeSet =
      CreateDefaultSubobject<UCombatAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent *ACombatPlayerState::GetAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

void ACombatPlayerState::BeginPlay() {
  Super::BeginPlay();

  if (AbilitySystemComponent) {
    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    // Set default attributes (server-only for multiplayer safety)
    if (HasAuthority()) {
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetMaxHealthAttribute(), DefaultMaxHP);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetHealthAttribute(), DefaultMaxHP);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetDamageAttribute(), DefaultMeleeDamage);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetKnockbackImpulseAttribute(),
          DefaultMeleeKnockbackImpulse);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetLaunchImpulseAttribute(),
          DefaultMeleeLaunchImpulse);
    }
  }
}
