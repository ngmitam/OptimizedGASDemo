// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"
#include "Attributes/StaminaAttributeSet.h"

UCombatStaminaComponent::UCombatStaminaComponent(
    const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer) {
  PrimaryComponentTick.bCanEverTick = false;
}

void UCombatStaminaComponent::OnRegister() {
  Super::OnRegister();

  // Get the stamina and max stamina attributes from the attribute set
  StaminaAttribute = UStaminaAttributeSet::GetStaminaAttribute();
  MaxStaminaAttribute = UStaminaAttributeSet::GetMaxStaminaAttribute();
}

void UCombatStaminaComponent::BeginPlay() {
  Super::BeginPlay();

  // Try to initialize with the owner's ability system
  AActor *Owner = GetOwner();
  if (Owner) {
    if (UAbilitySystemComponent *ASC =
            UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner)) {
      InitializeWithAbilitySystem(ASC);
    }
  }
}

void UCombatStaminaComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  if (AbilitySystemComponent) {
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(StaminaAttribute)
        .Remove(StaminaChangedDelegateHandle);
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(MaxStaminaAttribute)
        .Remove(MaxStaminaChangedDelegateHandle);
  }

  Super::EndPlay(EndPlayReason);
}

void UCombatStaminaComponent::InitializeWithAbilitySystem(
    UAbilitySystemComponent *ASC) {
  // Clean up old bindings if they exist
  if (AbilitySystemComponent) {
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(StaminaAttribute)
        .Remove(StaminaChangedDelegateHandle);
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(MaxStaminaAttribute)
        .Remove(MaxStaminaChangedDelegateHandle);
  }

  AbilitySystemComponent = ASC;

  if (AbilitySystemComponent) {
    // Bind to stamina attribute changes
    StaminaChangedDelegateHandle =
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(StaminaAttribute)
            .AddUObject(this, &UCombatStaminaComponent::HandleStaminaChanged);
    MaxStaminaChangedDelegateHandle =
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(MaxStaminaAttribute)
            .AddUObject(this,
                        &UCombatStaminaComponent::HandleMaxStaminaChanged);

    // Initial check
    if (IsDepleted()) {
      HandleDepleted();
    }
  }
}

float UCombatStaminaComponent::GetStamina() const {
  if (AbilitySystemComponent) {
    return AbilitySystemComponent->GetNumericAttribute(StaminaAttribute);
  }
  return 0.0f;
}

float UCombatStaminaComponent::GetMaxStamina() const {
  if (AbilitySystemComponent) {
    return AbilitySystemComponent->GetNumericAttribute(MaxStaminaAttribute);
  }
  return 0.0f;
}

bool UCombatStaminaComponent::IsDepleted() const {
  return GetStamina() <= 0.0f;
}

float UCombatStaminaComponent::GetStaminaNormalized() const {
  const float MaxStamina = GetMaxStamina();
  if (MaxStamina > 0.0f) {
    return GetStamina() / MaxStamina;
  }
  return 0.0f;
}

void UCombatStaminaComponent::HandleStaminaChanged(
    const FOnAttributeChangeData &ChangeData) {
  const float NewStamina = ChangeData.NewValue;
  const float OldStamina = ChangeData.OldValue;

  // Check for depletion/restored
  if (NewStamina <= 0.0f && OldStamina > 0.0f) {
    HandleDepleted();
  } else if (NewStamina > 0.0f && OldStamina <= 0.0f) {
    HandleRestored();
  }

  // Broadcast stamina changed
  OnStaminaChanged.Broadcast(NewStamina);
}

void UCombatStaminaComponent::HandleMaxStaminaChanged(
    const FOnAttributeChangeData &ChangeData) {
  const float NewMaxStamina = ChangeData.NewValue;

  // Broadcast max stamina changed
  OnMaxStaminaChanged.Broadcast(NewMaxStamina);
}

void UCombatStaminaComponent::HandleDepleted() {
  // Default depletion handling (override if needed)
}

void UCombatStaminaComponent::HandleRestored() {
  // Default restored handling (override if needed)
}
