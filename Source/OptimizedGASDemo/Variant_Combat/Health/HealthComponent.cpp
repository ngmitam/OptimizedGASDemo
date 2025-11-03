// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"
#include "Gameplay/Attributes/CombatAttributeSet.h"

UHealthComponent::UHealthComponent(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer) {
  PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::OnRegister() {
  Super::OnRegister();

  // Get the health and max health attributes from the attribute set
  if (UAbilitySystemGlobals::Get().IsAbilitySystemGlobalsInitialized()) {
    HealthAttribute =
        FGameplayAttribute(UCombatAttributeSet::GetHealthAttribute());
    MaxHealthAttribute =
        FGameplayAttribute(UCombatAttributeSet::GetMaxHealthAttribute());
  }
}

void UHealthComponent::BeginPlay() {
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

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  if (AbilitySystemComponent) {
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(HealthAttribute)
        .Remove(HealthChangedDelegateHandle);
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(MaxHealthAttribute)
        .Remove(MaxHealthChangedDelegateHandle);
  }

  Super::EndPlay(EndPlayReason);
}

void UHealthComponent::InitializeWithAbilitySystem(
    UAbilitySystemComponent *InASC) {
  AbilitySystemComponent = InASC;

  if (AbilitySystemComponent) {
    // Bind to health attribute changes
    HealthChangedDelegateHandle =
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(HealthAttribute)
            .AddUObject(this, &UHealthComponent::HandleHealthChanged);
    MaxHealthChangedDelegateHandle =
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(MaxHealthAttribute)
            .AddUObject(this, &UHealthComponent::HandleMaxHealthChanged);

    // Initial check
    if (IsDead()) {
      HandleDeath();
    }
  }
}

float UHealthComponent::GetHealth() const {
  if (AbilitySystemComponent) {
    return AbilitySystemComponent->GetNumericAttribute(HealthAttribute);
  }
  return 0.0f;
}

float UHealthComponent::GetMaxHealth() const {
  if (AbilitySystemComponent) {
    return AbilitySystemComponent->GetNumericAttribute(MaxHealthAttribute);
  }
  return 0.0f;
}

bool UHealthComponent::IsDead() const { return GetHealth() <= 0.0f; }

float UHealthComponent::GetHealthNormalized() const {
  const float MaxHealth = GetMaxHealth();
  if (MaxHealth > 0.0f) {
    return GetHealth() / MaxHealth;
  }
  return 0.0f;
}

void UHealthComponent::HandleHealthChanged(
    const FOnAttributeChangeData &ChangeData) {
  const float NewHealth = ChangeData.NewValue;
  const float OldHealth = ChangeData.OldValue;

  // Check for death
  if (NewHealth <= 0.0f && OldHealth > 0.0f) {
    HandleDeath();
  } else if (NewHealth > 0.0f && OldHealth <= 0.0f) {
    HandleRevive();
  }

  // Broadcast health changed
  OnHealthChanged.Broadcast(NewHealth);
}

void UHealthComponent::HandleMaxHealthChanged(
    const FOnAttributeChangeData &ChangeData) {
  const float NewMaxHealth = ChangeData.NewValue;

  // Broadcast max health changed
  OnMaxHealthChanged.Broadcast(NewMaxHealth);
}

void UHealthComponent::HandleDeath() {
  // Handle death logic here
  // For example, disable input, play death animation, etc.
  // This can be overridden in subclasses or use delegates
}

void UHealthComponent::HandleRevive() {
  // Handle revive logic here
  // For example, enable input, reset animations, etc.
}
