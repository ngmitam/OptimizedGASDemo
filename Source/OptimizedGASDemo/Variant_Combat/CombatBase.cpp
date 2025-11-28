// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatBase.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
#include "Abilities/CombatAbilitySet.h"
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatNotifyEnemiesAbility.h"
#include "Effects/CombatDamageGameplayEffect.h"
#include "Data/CombatAttackEventData.h"
#include "Data/CombatDamageEventData.h"
#include "UI/CombatLifeBar.h"

ACombatBase::ACombatBase() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatBase::AttackMontageEnded);
}

UAbilitySystemComponent *ACombatBase::GetAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

void ACombatBase::DoAttackTrace(FName DamageSourceBone) {
  // Create attack event data with damage source bone
  UCombatAttackEventData *AttackData = NewObject<UCombatAttackEventData>(this);
  AttackData->DamageSourceBone = DamageSourceBone;

  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Trace.Attack")), 0.0f,
      AttackData);
}

void ACombatBase::CheckCombo() {
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Next")));
}

void ACombatBase::CheckChargedAttack() {
  if (bIsChargingAttack) {
    SendGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Loop")));
  } else {
    SendGameplayEvent(FGameplayTag::RequestGameplayTag(
        FName("Event.Attack.Charged.Release")));
  }
}

void ACombatBase::ApplyDamage(float Damage, AActor *DamageCauser,
                              const FVector &DamageLocation,
                              const FVector &DamageImpulse) {
  UCombatDamageEventData *DamageData = NewObject<UCombatDamageEventData>();
  DamageData->Location = DamageLocation;
  DamageData->Impulse = DamageImpulse;

  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Damage.Received")), Damage,
      DamageData, DamageCauser);
}

void ACombatBase::HandleDeath() {
  SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName("Event.Death")));
}

void ACombatBase::ApplyHealing(float Healing, AActor *Healer) {
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Healing.Received")),
      Healing, nullptr, Healer);
}

void ACombatBase::NotifyDanger(const FVector &DangerLocation,
                               AActor *DangerSource) {
  LastDangerLocation = DangerLocation;
  LastDangerTime = GetWorld()->GetTimeSeconds();
}

void ACombatBase::AttackMontageEnded(UAnimMontage *Montage, bool bInterrupted) {
  // reset the attacking flag
  bIsAttacking = false;
}

void ACombatBase::Landed(const FHitResult &Hit) {
  Super::Landed(Hit);

  // reset the mesh transform to fix ragdoll physics
  if (GetMesh()) {
    GetMesh()->SetWorldTransform(GetMesh()->GetComponentTransform());
  }
}

void ACombatBase::BeginPlay() {
  Super::BeginPlay();

  // Initialize ability system components
  InitializeAbilitySystemComponents();

  // Initialize pawn data if available
  InitializePawnData();
}

void ACombatBase::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
}

void ACombatBase::InitializeAbilitySystemComponents() {
  if (!AbilitySystemComponent) {
    return;
  }

  AbilitySystemComponent->InitAbilityActorInfo(this, this);

  // Initialize health component
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent()) {
    if (HealthComponent) {
      HealthComponent->InitializeWithAbilitySystem(ASC);
    }
  }

  // Bind to health component delegates for UI updates
  if (HealthComponent) {
    HealthComponent->OnHealthChanged.AddUObject(
        this, &ACombatBase::OnHealthComponentChanged);
    HealthComponent->OnMaxHealthChanged.AddUObject(
        this, &ACombatBase::OnMaxHealthComponentChanged);
  }
}

void ACombatBase::InitializePawnData() {
  if (!PawnData) {
    return;
  }

  if (!AbilitySystemComponent) {
    return;
  }

  // Load from data table if specified
  PawnData->LoadFromDataTable();

  // Grant ability sets (server-only for multiplayer safety)
  if (HasAuthority()) {
    bool bHasAbilitySets = false;
    for (const FCombatAbilitySetWithInput &AbilitySetWithInput :
         PawnData->AbilitySets) {
      if (AbilitySetWithInput.AbilitySet) {
        FCombatAbilitySetHandle AbilitySetHandle;
        AbilitySetWithInput.AbilitySet->GiveToAbilitySystem(
            AbilitySystemComponent, AbilitySetHandle, this);
        bHasAbilitySets = true;
        // Store the handle for later removal if needed
        // Note: In a full implementation, you'd store these handles
      }
    }

    // Fallback: If no ability sets are configured, add attribute sets directly
    if (!bHasAbilitySets) {
      for (TSubclassOf<UAttributeSet> AttributeSetClass :
           PawnData->AttributeSets) {
        if (AttributeSetClass) {
          UAttributeSet *NewAttributeSet = NewObject<UAttributeSet>(
              AbilitySystemComponent->GetOwner(), AttributeSetClass);
          AbilitySystemComponent->AddAttributeSetSubobject(NewAttributeSet);
        }
      }

      // Fallback: Grant abilities directly if no ability sets
      for (TSubclassOf<UGameplayAbility> AbilityClass :
           PawnData->GrantedAbilities) {
        if (AbilityClass) {
          FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
          AbilitySystemComponent->GiveAbility(AbilitySpec);
        }
      }

      // Fallback: Apply granted effects directly
      for (TSubclassOf<UGameplayEffect> EffectClass :
           PawnData->GrantedEffects) {
        if (EffectClass) {
          FGameplayEffectSpecHandle EffectSpecHandle =
              AbilitySystemComponent->MakeOutgoingSpec(
                  EffectClass, DefaultEffectLevel,
                  AbilitySystemComponent->MakeEffectContext());
          if (EffectSpecHandle.IsValid()) {
            AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
                *EffectSpecHandle.Data.Get(), AbilitySystemComponent);
          }
        }
      }
    }
  }
}

void ACombatBase::SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude,
                                    UObject *OptionalObject,
                                    AActor *EventInstigator) {
  if (!AbilitySystemComponent) {
    return;
  }

  FGameplayEventData EventData;
  EventData.EventMagnitude = EventMagnitude;
  EventData.Instigator = EventInstigator ? EventInstigator : this;
  EventData.Target = this;
  EventData.OptionalObject = OptionalObject;

  AbilitySystemComponent->HandleGameplayEvent(EventTag, &EventData);
}

void ACombatBase::OnHealthComponentChanged(float NewHealth) {
  CurrentHP = NewHealth;
  if (LifeBarWidget && HealthComponent) {
    float MaxHealth = HealthComponent->GetMaxHealth();
    float Percentage = CurrentHP / MaxHealth;
    LifeBarWidget->SetLifePercentage(Percentage);
  }
}

void ACombatBase::OnMaxHealthComponentChanged(float NewMaxHealth) {
  MaxHP = NewMaxHealth;
  if (LifeBarWidget && HealthComponent) {
    LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);
  }
}

void ACombatBase::ResetHP() {
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent()) {
    if (HealthComponent) {
      float MaxHealth = HealthComponent->GetMaxHealth();
      ASC->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(),
                                   MaxHealth);
    }
  }

  // update the life bar
  LifeBarWidget->SetLifePercentage(1.0f);
}

void ACombatBase::RemoveFromLevel() { Destroy(); }

void ACombatBase::NotifyEnemiesOfAttack() {
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Notify.Enemies")));
}

void ACombatBase::ReceivedDamage_Implementation(
    float Damage, const FVector &ImpactPoint, const FVector &DamageDirection) {
  // Default implementation does nothing, can be overridden in Blueprints
}

void ACombatBase::DealtDamage_Implementation(float Damage,
                                             const FVector &ImpactPoint) {
  // Default implementation does nothing, can be overridden in Blueprints
}
