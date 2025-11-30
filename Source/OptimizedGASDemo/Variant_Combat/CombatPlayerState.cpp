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
  StaminaAttributeSet =
      CreateDefaultSubobject<UStaminaAttributeSet>(TEXT("StaminaAttributeSet"));
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

  // Fallback: Grant legacy abilities if no AbilitySets are configured
  if (InPawnData->AbilitySets.IsEmpty()) {
    for (TSubclassOf<UGameplayAbility> AbilityClass :
         InPawnData->GrantedAbilities) {
      if (AbilityClass) {
        FGameplayAbilitySpecHandle AbilityHandle =
            AbilitySystemComponent->GiveAbility(
                FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
        if (AbilityHandle.IsValid()) {
          GrantedAbilitySpecHandles.Add(AbilityHandle);
        }
      }
    }

    // Fallback: Grant legacy effects if no AbilitySets are configured
    for (TSubclassOf<UGameplayEffect> EffectClass :
         InPawnData->GrantedEffects) {
      if (EffectClass) {
        FGameplayEffectSpecHandle EffectHandle =
            AbilitySystemComponent->MakeOutgoingSpec(
                EffectClass, 1.0f, AbilitySystemComponent->MakeEffectContext());
        if (EffectHandle.IsValid()) {
          FActiveGameplayEffectHandle ActiveHandle =
              AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
                  *EffectHandle.Data.Get(), AbilitySystemComponent);
          if (ActiveHandle.IsValid()) {
            GrantedEffectHandles.Add(ActiveHandle);
          }
        }
      }
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

  // Remove legacy granted abilities
  for (FGameplayAbilitySpecHandle &AbilityHandle : GrantedAbilitySpecHandles) {
    if (AbilityHandle.IsValid()) {
      AbilitySystemComponent->ClearAbility(AbilityHandle);
    }
  }

  GrantedAbilitySpecHandles.Reset();

  // Remove legacy granted effects
  for (FActiveGameplayEffectHandle &EffectHandle : GrantedEffectHandles) {
    if (EffectHandle.IsValid()) {
      AbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle);
    }
  }

  GrantedEffectHandles.Reset();
}
