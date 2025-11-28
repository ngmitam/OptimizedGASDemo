// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"

ACombatPlayerState::ACombatPlayerState() {
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));
  AbilitySystemComponent->SetIsReplicated(true);

  HealthAttributeSet =
      CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
  DamageAttributeSet =
      CreateDefaultSubobject<UDamageAttributeSet>(TEXT("DamageAttributeSet"));

  // Add attribute sets to ASC
  // Note: Attribute sets are now added from AbilitySets in PawnData or as
  // fallback
  // AbilitySystemComponent->AddAttributeSetSubobject(HealthAttributeSet);
  // AbilitySystemComponent->AddAttributeSetSubobject(DamageAttributeSet);
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
          UHealthAttributeSet::GetMaxHealthAttribute(), DefaultMaxHP);
      AbilitySystemComponent->SetNumericAttributeBase(
          UHealthAttributeSet::GetHealthAttribute(), DefaultMaxHP);
      AbilitySystemComponent->SetNumericAttributeBase(
          UDamageAttributeSet::GetDamageAttribute(), DefaultMeleeDamage);
      AbilitySystemComponent->SetNumericAttributeBase(
          UDamageAttributeSet::GetKnockbackImpulseAttribute(),
          DefaultMeleeKnockbackImpulse);
      AbilitySystemComponent->SetNumericAttributeBase(
          UDamageAttributeSet::GetLaunchImpulseAttribute(),
          DefaultMeleeLaunchImpulse);
    }
  }
}
