// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
#include "Attributes/MovementAttributeSet.h"

ACombatPlayerState::ACombatPlayerState() {
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));
  AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent *ACombatPlayerState::GetAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

void ACombatPlayerState::BeginPlay() {
  Super::BeginPlay();

  if (AbilitySystemComponent) {
    AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
  }
}

void ACombatPlayerState::SetPawnData(const UCombatPawnData *InPawnData) {
  if (!AbilitySystemComponent) {
    return;
  }

  // Clear any previously granted abilities/effects
  ClearPawnData();

  PawnData = InPawnData;

  if (!InPawnData) {
    return;
  }

  // Grant ability sets with input mapping
  for (const FCombatAbilitySetWithInput &AbilitySetWithInput :
       InPawnData->AbilitySets) {
    if (AbilitySetWithInput.AbilitySet) {
      FCombatAbilitySetHandle AbilitySetHandle;
      AbilitySetWithInput.AbilitySet->GiveToAbilitySystem(
          AbilitySystemComponent, AbilitySetHandle, this);
      if (AbilitySetHandle.IsValid()) {
        GrantedAbilitySetHandles.Add(AbilitySetHandle);
      }
    }
  }

  // Add attribute sets from PawnData
  for (TSubclassOf<UAttributeSet> AttributeSetClass :
       InPawnData->AttributeSets) {
    if (AttributeSetClass) {
      UAttributeSet *NewAttributeSet =
          NewObject<UAttributeSet>(this, AttributeSetClass);
      AbilitySystemComponent->AddAttributeSetSubobject(NewAttributeSet);
    }
  }

  // Set attributes from pawn data
  if (HasAuthority()) {
    AbilitySystemComponent->SetNumericAttributeBase(
        UHealthAttributeSet::GetHealthAttribute(), InPawnData->DefaultHealth);
    AbilitySystemComponent->SetNumericAttributeBase(
        UHealthAttributeSet::GetMaxHealthAttribute(),
        InPawnData->DefaultMaxHealth);
    AbilitySystemComponent->SetNumericAttributeBase(
        UStaminaAttributeSet::GetStaminaAttribute(),
        InPawnData->DefaultStamina);
    AbilitySystemComponent->SetNumericAttributeBase(
        UStaminaAttributeSet::GetMaxStaminaAttribute(),
        InPawnData->DefaultMaxStamina);
    AbilitySystemComponent->SetNumericAttributeBase(
        UDamageAttributeSet::GetDamageAttribute(), InPawnData->DefaultDamage);
    AbilitySystemComponent->SetNumericAttributeBase(
        UDamageAttributeSet::GetKnockbackImpulseAttribute(),
        InPawnData->DefaultKnockbackImpulse);
    AbilitySystemComponent->SetNumericAttributeBase(
        UDamageAttributeSet::GetLaunchImpulseAttribute(),
        InPawnData->DefaultLaunchImpulse);
  }

  // Update default values from pawn data
  DefaultMaxHP = InPawnData->DefaultMaxHealth;
  DefaultMaxStamina = InPawnData->DefaultMaxStamina;
}

void ACombatPlayerState::ClearPawnData() {
  if (!AbilitySystemComponent) {
    return;
  }

  // Remove all granted ability sets
  for (FCombatAbilitySetHandle &AbilitySetHandle : GrantedAbilitySetHandles) {
    if (AbilitySetHandle.IsValid()) {
      AbilitySetHandle.TakeFromAbilitySystem(AbilitySystemComponent);
    }
  }

  GrantedAbilitySetHandles.Reset();
  PawnData = nullptr;
}

void ACombatPlayerState::ResetAttributesToDefault() {
  if (!AbilitySystemComponent) {
    return;
  }

  // Set base attributes to default values
  AbilitySystemComponent->SetNumericAttributeBase(
      UHealthAttributeSet::GetHealthAttribute(), DefaultMaxHP);
  AbilitySystemComponent->SetNumericAttributeBase(
      UHealthAttributeSet::GetMaxHealthAttribute(), DefaultMaxHP);
  AbilitySystemComponent->SetNumericAttributeBase(
      UStaminaAttributeSet::GetStaminaAttribute(), DefaultMaxStamina);
  AbilitySystemComponent->SetNumericAttributeBase(
      UStaminaAttributeSet::GetMaxStaminaAttribute(), DefaultMaxStamina);

  // Override current values to ensure they are reset
  AbilitySystemComponent->ApplyModToAttribute(
      UHealthAttributeSet::GetHealthAttribute(), EGameplayModOp::Override,
      DefaultMaxHP);
  AbilitySystemComponent->ApplyModToAttribute(
      UStaminaAttributeSet::GetStaminaAttribute(), EGameplayModOp::Override,
      DefaultMaxStamina);
}
