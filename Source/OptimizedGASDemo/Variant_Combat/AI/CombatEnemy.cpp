// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatEnemy.h"
#include "CombatCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatAIController.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "CombatLifeBar.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/Attributes/CombatAttributeSet.h"
#include "GameplayTagsManager.h"
#include "Gameplay/Abilities/CombatReceiveDamageAbility.h"
#include "Gameplay/Abilities/CombatDeathAbility.h"
#include "Gameplay/Abilities/CombatTraceAttackAbility.h"
#include "Gameplay/Abilities/CombatChargedAttackAbility.h"
#include "Gameplay/Abilities/CombatComboAttackAbility.h"
#include "Gameplay/Abilities/CombatNotifyEnemiesAbility.h"
#include "Gameplay/Effects/CombatDamageGameplayEffect.h"
#include "Gameplay/Data/AttackEventData.h"

ACombatEnemy::ACombatEnemy() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatEnemy::AttackMontageEnded);

  // set the AI Controller class by default
  AIControllerClass = ACombatAIController::StaticClass();

  // use an AI Controller regardless of whether we're placed or spawned
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

  // ignore the controller's yaw rotation
  bUseControllerRotationYaw = false;

  // create the life bar
  LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
  LifeBar->SetupAttachment(RootComponent);

  // create the health component
  HealthComponent =
      CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

  // create the ability system component
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));
  AbilitySystemComponent->SetIsReplicated(
      false); // Enemies don't need replication

  // create the attribute set
  AttributeSet =
      CreateDefaultSubobject<UCombatAttributeSet>(TEXT("AttributeSet"));

  // Create default pawn data with abilities
  PawnData = CreateDefaultSubobject<UCombatPawnData>(TEXT("PawnData"));
  if (PawnData) {
    // Set default attributes
    PawnData->DefaultHealth = MaxHP;
    PawnData->DefaultMaxHealth = MaxHP;
    PawnData->DefaultDamage = MeleeDamage;
    PawnData->DefaultKnockbackImpulse = MeleeKnockbackImpulse;
    PawnData->DefaultLaunchImpulse = MeleeLaunchImpulse;
    PawnData->DefaultTraceDistance = MeleeTraceDistance;
    PawnData->DefaultTraceRadius = MeleeTraceRadius;

    // Add granted abilities
    PawnData->GrantedAbilities.Add(UCombatReceiveDamageAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatDeathAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatTraceAttackAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatChargedAttackAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatComboAttackAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatNotifyEnemiesAbility::StaticClass());
  }

  // set the collision capsule size
  GetCapsuleComponent()->SetCapsuleSize(35.0f, 90.0f);

  // set the character movement properties
  GetCharacterMovement()->bUseControllerDesiredRotation = true;

  // reset HP to maximum
  CurrentHP = MaxHP;
}

UAbilitySystemComponent *ACombatEnemy::GetAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

void ACombatEnemy::DoAIComboAttack() {
  // ignore if we're already playing an attack animation
  if (bIsAttacking) {
    return;
  }

  // Activate combo attack ability
  if (AbilitySystemComponent) {
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(
        FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack.Combo")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
  }
}

void ACombatEnemy::DoAIChargedAttack() {
  // ignore if we're already playing an attack animation
  if (bIsAttacking) {
    return;
  }

  // Activate charged attack ability
  if (AbilitySystemComponent) {
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(
        FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack.Charged")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
  }
}

void ACombatEnemy::AttackMontageEnded(UAnimMontage *Montage,
                                      bool bInterrupted) {
  // reset the attacking flag
  bIsAttacking = false;

  // call the attack completed delegate so the StateTree can continue execution
  OnAttackCompleted.ExecuteIfBound();
}

const FVector &ACombatEnemy::GetLastDangerLocation() const {
  return LastDangerLocation;
}

float ACombatEnemy::GetLastDangerTime() const { return LastDangerTime; }

void ACombatEnemy::DoAttackTrace(FName DamageSourceBone) {
  // Send gameplay event to activate attack trace ability
  if (AbilitySystemComponent) {
    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.Target = this;

    // Create attack event data with damage source bone
    UAttackEventData *AttackData = NewObject<UAttackEventData>(this);
    AttackData->DamageSourceBone = DamageSourceBone;
    EventData.OptionalObject = AttackData;

    AbilitySystemComponent->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Attack.Trace")),
        &EventData);
  }
}

void ACombatEnemy::CheckCombo() {
  // Send gameplay event to advance combo
  if (AbilitySystemComponent) {
    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.Target = this;

    AbilitySystemComponent->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Next")),
        &EventData);
  }
}

void ACombatEnemy::CheckChargedAttack() {
  // Send gameplay event for charged attack check
  if (AbilitySystemComponent) {
    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.Target = this;

    AbilitySystemComponent->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Loop")),
        &EventData);
  }
}

void ACombatEnemy::ApplyDamage(float Damage, AActor *DamageCauser,
                               const FVector &DamageLocation,
                               const FVector &DamageImpulse) {
  // Send gameplay event to activate receive damage ability
  if (AbilitySystemComponent) {
    FGameplayEventData EventData;
    EventData.EventMagnitude = Damage;
    EventData.Instigator = DamageCauser;
    EventData.Target = this;

    UDamageEventData *DamageData = NewObject<UDamageEventData>();
    DamageData->Location = DamageLocation;
    DamageData->Impulse = DamageImpulse;
    EventData.OptionalObject = DamageData;

    AbilitySystemComponent->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Damage.Received")),
        &EventData);
  }
}

void ACombatEnemy::HandleDeath() {
  // Activate death ability
  if (AbilitySystemComponent) {
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(
        FGameplayTag::RequestGameplayTag(FName("Ability.Type.Death")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
  }

  // hide the life bar
  LifeBar->SetHiddenInGame(true);

  // enable full ragdoll physics
  GetMesh()->SetSimulatePhysics(true);

  // call the died delegate to notify any subscribers
  OnEnemyDied.Broadcast();
}

void ACombatEnemy::ApplyHealing(float Healing, AActor *Healer) {
  // stub
}

void ACombatEnemy::Landed(const FHitResult &Hit) {
  Super::Landed(Hit);

  // is the character still alive?
  if (HealthComponent && !HealthComponent->IsDead()) {
    // disable ragdoll physics
    GetMesh()->SetPhysicsBlendWeight(0.0f);
  }

  // call the landed Delegate for StateTree
  OnEnemyLanded.ExecuteIfBound();
}

void ACombatEnemy::BeginPlay() {
  // reset HP to maximum (keep this for StateTree compatibility)
  CurrentHP = MaxHP;

  Super::BeginPlay();

  // get the life bar widget from the widget comp
  LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
  check(LifeBarWidget);

  // Initialize GAS
  if (AbilitySystemComponent) {
    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    if (PawnData) {
      // Load from data table if specified
      PawnData->LoadFromDataTable();

      // Set default attributes from PawnData
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetHealthAttribute(), PawnData->DefaultHealth);
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetMaxHealthAttribute(), PawnData->DefaultMaxHealth);
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetDamageAttribute(), PawnData->DefaultDamage);
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetKnockbackImpulseAttribute(),
          PawnData->DefaultKnockbackImpulse);
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetLaunchImpulseAttribute(),
          PawnData->DefaultLaunchImpulse);
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetTraceDistanceAttribute(),
          PawnData->DefaultTraceDistance);
      AbilitySystemComponent->SetNumericAttributeBase(
          AttributeSet->GetTraceRadiusAttribute(),
          PawnData->DefaultTraceRadius);

      // Grant abilities
      for (TSubclassOf<UGameplayAbility> AbilityClass :
           PawnData->GrantedAbilities) {
        if (AbilityClass) {
          FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
          AbilitySystemComponent->GiveAbility(AbilitySpec);
        }
      }

      // Apply granted effects
      for (TSubclassOf<UGameplayEffect> EffectClass :
           PawnData->GrantedEffects) {
        if (EffectClass) {
          FGameplayEffectSpecHandle EffectSpecHandle =
              AbilitySystemComponent->MakeOutgoingSpec(
                  EffectClass, 1.0f,
                  AbilitySystemComponent->MakeEffectContext());
          if (EffectSpecHandle.IsValid()) {
            AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
                *EffectSpecHandle.Data.Get(), AbilitySystemComponent);
          }
        }
      }
    } else {
      // Fallback to old initialization
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetMaxHealthAttribute(), MaxHP);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetHealthAttribute(), CurrentHP);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetDamageAttribute(), MeleeDamage);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetKnockbackImpulseAttribute(),
          MeleeKnockbackImpulse);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetLaunchImpulseAttribute(), MeleeLaunchImpulse);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetTraceDistanceAttribute(), MeleeTraceDistance);
      AbilitySystemComponent->SetNumericAttributeBase(
          UCombatAttributeSet::GetTraceRadiusAttribute(), MeleeTraceRadius);
    }

    // Initialize health component
    HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);

    // Bind to health component delegates to sync CurrentHP
    HealthComponent->OnHealthChanged.AddUObject(
        this, &ACombatEnemy::OnHealthComponentChanged);
    HealthComponent->OnMaxHealthChanged.AddUObject(
        this, &ACombatEnemy::OnMaxHealthComponentChanged);
  }

  // fill the life bar
  LifeBarWidget->SetLifePercentage(1.0f);
}

void ACombatEnemy::EndPlay(EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  // clear the death timer
  GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

void ACombatEnemy::OnHealthComponentChanged(float NewHealth) {
  CurrentHP = NewHealth;
  if (LifeBarWidget) {
    float MaxHealth = HealthComponent->GetMaxHealth();
    LifeBarWidget->SetLifePercentage(CurrentHP / MaxHealth);
  }
}

void ACombatEnemy::OnMaxHealthComponentChanged(float NewMaxHealth) {
  MaxHP = NewMaxHealth;
  if (LifeBarWidget) {
    LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);
  }
}

void ACombatEnemy::NotifyDanger(const FVector &DangerLocation,
                                AActor *DangerSource) {
  LastDangerLocation = DangerLocation;
  LastDangerTime = GetWorld()->GetTimeSeconds();
}

void ACombatEnemy::RemoveFromLevel() { Destroy(); }
