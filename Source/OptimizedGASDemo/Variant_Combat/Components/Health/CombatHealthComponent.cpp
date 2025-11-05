// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"
#include "Gameplay/Attributes/CombatAttributeSet.h"

UCombatHealthComponent::UCombatHealthComponent(
    const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer) {
  PrimaryComponentTick.bCanEverTick = false;
}

void UCombatHealthComponent::OnRegister() {
  Super::OnRegister();

  // Get the health and max health attributes from the attribute set
  HealthAttribute = UCombatAttributeSet::GetHealthAttribute();
  MaxHealthAttribute = UCombatAttributeSet::GetMaxHealthAttribute();
}

void UCombatHealthComponent::BeginPlay() {
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

void UCombatHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
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

void UCombatHealthComponent::InitializeWithAbilitySystem(
    UAbilitySystemComponent *InASC) {
  AbilitySystemComponent = InASC;

  if (AbilitySystemComponent) {
    // Bind to health attribute changes
    HealthChangedDelegateHandle =
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(HealthAttribute)
            .AddUObject(this, &UCombatHealthComponent::HandleHealthChanged);
    MaxHealthChangedDelegateHandle =
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(MaxHealthAttribute)
            .AddUObject(this, &UCombatHealthComponent::HandleMaxHealthChanged);

    // Initial check
    if (IsDead()) {
      HandleDeath();
    }
  }
}

float UCombatHealthComponent::GetHealth() const {
  if (AbilitySystemComponent) {
    return AbilitySystemComponent->GetNumericAttribute(HealthAttribute);
  }
  return 0.0f;
}

float UCombatHealthComponent::GetMaxHealth() const {
  if (AbilitySystemComponent) {
    return AbilitySystemComponent->GetNumericAttribute(MaxHealthAttribute);
  }
  return 0.0f;
}

bool UCombatHealthComponent::IsDead() const { return GetHealth() <= 0.0f; }

float UCombatHealthComponent::GetHealthNormalized() const {
  const float MaxHealth = GetMaxHealth();
  if (MaxHealth > 0.0f) {
    return GetHealth() / MaxHealth;
  }
  return 0.0f;
}

void UCombatHealthComponent::HandleHealthChanged(
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

void UCombatHealthComponent::HandleMaxHealthChanged(
    const FOnAttributeChangeData &ChangeData) {
  const float NewMaxHealth = ChangeData.NewValue;

  // Broadcast max health changed
  OnMaxHealthChanged.Broadcast(NewMaxHealth);
}

void UCombatHealthComponent::HandleDeath() {
  // Handle death logic here
  // For example, disable input, play death animation, etc.
  // This can be overridden in subclasses or use delegates

  // Enable ragdoll physics on death
  AActor *Owner = GetOwner();
  if (Owner) {
    ACharacter *Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh()) {
      Character->GetMesh()->SetPhysicsBlendWeight(1.0f);
      Character->GetMesh()->SetSimulatePhysics(true);
    }
  }
}

void UCombatHealthComponent::HandleRevive() {
  // Handle revive logic here
  // For example, enable input, reset animations, etc.
}
