// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
#include "Gameplay/Attributes/StaminaAttributeSet.h"
#include "Gameplay/Attributes/MovementAttributeSet.h"
#include "Abilities/CombatAbilitySet.h"
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatStaminaRegenerationAbility.h"
#include "Effects/CombatStaminaCostGameplayEffect.h"
#include "Data/CombatAttackEventData.h"
#include "Data/CombatDamageEventData.h"
#include "UI/CombatLifeBar.h"

ACombatBase::ACombatBase() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatBase::AttackMontageEnded);

  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

  // create the life bar widget component
  LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
  LifeBar->SetupAttachment(RootComponent);

  // create the health component
  HealthComponent =
      CreateDefaultSubobject<UCombatHealthComponent>(TEXT("HealthComponent"));

  // create the stamina component
  StaminaComponent =
      CreateDefaultSubobject<UCombatStaminaComponent>(TEXT("StaminaComponent"));

  // Create default pawn data with abilities
  PawnData = CreateDefaultSubobject<UCombatPawnData>(TEXT("PawnData"));
  if (PawnData) {
    // Set attribute sets
    PawnData->AttributeSets.Add(UHealthAttributeSet::StaticClass());
    PawnData->AttributeSets.Add(UDamageAttributeSet::StaticClass());
    PawnData->AttributeSets.Add(UStaminaAttributeSet::StaticClass());
    PawnData->AttributeSets.Add(UMovementAttributeSet::StaticClass());

    // Set default attributes
    PawnData->DefaultHealth = MaxHP;
    PawnData->DefaultMaxHealth = MaxHP;
    PawnData->DefaultStamina = 100.0f;
    PawnData->DefaultMaxStamina = 100.0f;
    PawnData->DefaultDamage = MeleeDamage;
    PawnData->DefaultKnockbackImpulse = MeleeKnockbackImpulse;
    PawnData->DefaultLaunchImpulse = MeleeLaunchImpulse;

    // Note: AbilitySets should be configured in the editor or derived classes
    // Note: Lock ability is set in subclasses (LockToggle for player, Lockable
    // for enemy)
  }
}

UAbilitySystemComponent *ACombatBase::GetAbilitySystemComponent() const {
  return nullptr;
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
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (ASC && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(
                 FName("State.ChargingAttack")))) {
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
  // Tags are automatically removed when ability ends
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

  // get the life bar widget from the widget comp
  LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
  check(LifeBarWidget);

  // fill the life bar
  LifeBarWidget->SetLifePercentage(1.0f);

  // Bind to health component delegates for UI updates
  if (HealthComponent) {
    HealthComponent->OnHealthChanged.AddUObject(
        this, &ACombatBase::OnHealthComponentChanged);
    HealthComponent->OnMaxHealthChanged.AddUObject(
        this, &ACombatBase::OnMaxHealthComponentChanged);
  }

  // Bind to stamina component delegates
  if (StaminaComponent) {
    StaminaComponent->OnStaminaChanged.AddUObject(
        this, &ACombatBase::OnStaminaComponentChanged);
    StaminaComponent->OnMaxStaminaChanged.AddUObject(
        this, &ACombatBase::OnMaxStaminaComponentChanged);
  }
}

void ACombatBase::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
}

void ACombatBase::SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude,
                                    UObject *OptionalObject,
                                    AActor *EventInstigator) {
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (!ASC) {
    return;
  }

  FGameplayEventData EventData;
  EventData.EventMagnitude = EventMagnitude;
  EventData.Instigator = EventInstigator ? EventInstigator : this;
  EventData.Target = this;
  EventData.OptionalObject = OptionalObject;

  ASC->HandleGameplayEvent(EventTag, &EventData);
}

void ACombatBase::OnHealthComponentChanged(float NewHealth) {
  CurrentHP = NewHealth;
  if (LifeBarWidget && HealthComponent) {
    float MaxHealth = HealthComponent->GetMaxHealth();
    float Percentage = CurrentHP / MaxHealth;
    LifeBarWidget->SetLifePercentage(Percentage);
  }
}

void ACombatBase::OnStaminaComponentChanged(float NewStamina) {
  CurrentStamina = NewStamina;
  if (LifeBarWidget && StaminaComponent) {
    float MaxStamina = StaminaComponent->GetMaxStamina();
    float Percentage = CurrentStamina / MaxStamina;
    LifeBarWidget->SetStaminaPercentage(Percentage);
  }
}

void ACombatBase::OnMaxStaminaComponentChanged(float NewMaxStamina) {
  // If there is a stamina UI, update it. For now no-op like health does for
  // life bar
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

void ACombatBase::ResetStamina() {
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent()) {
    if (StaminaComponent) {
      float MaxStamina = StaminaComponent->GetMaxStamina();
      ASC->SetNumericAttributeBase(UStaminaAttributeSet::GetStaminaAttribute(),
                                   MaxStamina);
    }
  }
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

void ACombatBase::HandleMovementSpeedChanged(
    const FOnAttributeChangeData &Data) {
  // Update the character's movement speed based on the GAS attribute
  if (GetCharacterMovement()) {
    GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
  }
}

void ACombatBase::InitializeMovementAttributes(UAbilitySystemComponent *ASC) {
  if (!ASC) {
    return;
  }

  // Initialize movement speed attributes
  ASC->SetNumericAttributeBase(
      UMovementAttributeSet::GetMovementSpeedAttribute(),
      GetCharacterMovement()->GetMaxSpeed());
  ASC->SetNumericAttributeBase(
      UMovementAttributeSet::GetMaxMovementSpeedAttribute(),
      GetCharacterMovement()->GetMaxSpeed());

  // Bind movement speed attribute change delegate
  ASC->GetGameplayAttributeValueChangeDelegate(
         UMovementAttributeSet::GetMovementSpeedAttribute())
      .AddUObject(this, &ACombatBase::HandleMovementSpeedChanged);
}
