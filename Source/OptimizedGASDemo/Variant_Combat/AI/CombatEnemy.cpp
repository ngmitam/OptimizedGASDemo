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
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Attributes/MovementAttributeSet.h"
#include "GameplayTagsManager.h"
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatNotifyEnemiesAbility.h"
#include "Abilities/CombatLockableAbility.h"
#include "Abilities/CombatLockToggleAbility.h"
#include "Abilities/CombatLockToggleAbility.h"
#include "Effects/CombatDamageGameplayEffect.h"
#include "Data/CombatAttackEventData.h"
#include "Data/CombatPawnData.h"
#include "Abilities/CombatAbilitySet.h"

/** Constructor */
ACombatEnemy::ACombatEnemy() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatEnemy::AttackMontageEnded);

  // create the ability system component
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));

  // create the attribute sets
  HealthAttributeSet =
      CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
  DamageAttributeSet =
      CreateDefaultSubobject<UDamageAttributeSet>(TEXT("DamageAttributeSet"));
  StaminaAttributeSet =
      CreateDefaultSubobject<UStaminaAttributeSet>(TEXT("StaminaAttributeSet"));
  MovementAttributeSet = CreateDefaultSubobject<UMovementAttributeSet>(
      TEXT("MovementAttributeSet"));

  // Add lockable ability for enemy
  if (PawnData) {
    PawnData->GrantedAbilities.Add(UCombatLockableAbility::StaticClass());
    PawnData->DefaultDamage = 10.0f;
  }

  // set the AI Controller class by default
  AIControllerClass = ACombatAIController::StaticClass();

  // use an AI Controller regardless of whether we're placed or spawned
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

  // ignore the controller's yaw rotation
  bUseControllerRotationYaw = false;

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
  // Send gameplay event to activate combo attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Start")));
}

void ACombatEnemy::DoAIChargedAttack() {
  // Send gameplay event to activate charged attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Start")));
}

void ACombatEnemy::AttackMontageEnded(UAnimMontage *Montage,
                                      bool bInterrupted) {
  Super::AttackMontageEnded(Montage, bInterrupted);

  // call the attack completed delegate so the StateTree can continue execution
  OnAttackCompleted.ExecuteIfBound();
}

void ACombatEnemy::HandleDeath() {
  // Send death event to activate death ability
  Super::HandleDeath();

  // hide the life bar
  LifeBar->SetHiddenInGame(true);

  // call the died delegate to notify any subscribers
  OnEnemyDied.Broadcast();
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
  // Initialize ability system components
  InitializeAbilitySystemComponents();

  // Initialize pawn data if available
  InitializePawnData();

  Super::BeginPlay();
}

void ACombatEnemy::EndPlay(EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  // clear the death timer
  GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

void ACombatEnemy::InitializeAbilitySystemComponents() {
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (!ASC) {
    return;
  }

  if (!ASC->GetAvatarActor_Direct()) {
    ASC->InitAbilityActorInfo(this, this);
  }

  // Add attribute sets to the ASC
  if (HealthAttributeSet) {
    ASC->AddAttributeSetSubobject(HealthAttributeSet);
  }
  if (DamageAttributeSet) {
    ASC->AddAttributeSetSubobject(DamageAttributeSet);
  }
  if (StaminaAttributeSet) {
    ASC->AddAttributeSetSubobject(StaminaAttributeSet);
  }
  if (MovementAttributeSet) {
    ASC->AddAttributeSetSubobject(MovementAttributeSet);
  }

  // Initialize health component
  if (HealthComponent) {
    HealthComponent->InitializeWithAbilitySystem(ASC);
  }

  // Initialize stamina component
  if (StaminaComponent) {
    StaminaComponent->InitializeWithAbilitySystem(ASC);
  }

  // Initialize movement attributes and bind delegate
  InitializeMovementAttributes(ASC);
}

void ACombatEnemy::InitializePawnData() {
  if (!PawnData) {
    return;
  }

  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (!ASC) {
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
            ASC, AbilitySetHandle, this);
        bHasAbilitySets = true;
      }
    }

    // Fallback: If no ability sets are configured, add attribute sets directly
    if (!bHasAbilitySets) {
      for (TSubclassOf<UAttributeSet> AttributeSetClass :
           PawnData->AttributeSets) {
        if (AttributeSetClass) {
          UAttributeSet *NewAttributeSet =
              NewObject<UAttributeSet>(ASC->GetOwner(), AttributeSetClass);
          ASC->AddAttributeSetSubobject(NewAttributeSet);
        }
      }

      // Fallback: Grant abilities directly if no ability sets (only if not
      // already granted)
      for (TSubclassOf<UGameplayAbility> AbilityClass :
           PawnData->GrantedAbilities) {
        if (AbilityClass && !ASC->FindAbilitySpecFromClass(AbilityClass)) {
          FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
          ASC->GiveAbility(AbilitySpec);
        }
      }

      // Fallback: Apply granted effects directly (only if not already active)
      for (TSubclassOf<UGameplayEffect> EffectClass :
           PawnData->GrantedEffects) {
        if (EffectClass) {
          FGameplayEffectQuery Query;
          Query.EffectDefinition = EffectClass;
          TArray<FActiveGameplayEffectHandle> ActiveEffects =
              ASC->GetActiveEffects(Query);
          if (ActiveEffects.Num() == 0) {
            FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(
                EffectClass, 1.0f, ASC->MakeEffectContext());
            if (EffectSpecHandle.IsValid()) {
              ASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),
                                                   ASC);
            }
          }
        }
      }

      // Set attributes from pawn data
      ASC->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(),
                                   PawnData->DefaultHealth);
      ASC->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(),
                                   PawnData->DefaultMaxHealth);
      ASC->SetNumericAttributeBase(UStaminaAttributeSet::GetStaminaAttribute(),
                                   PawnData->DefaultStamina);
      ASC->SetNumericAttributeBase(
          UStaminaAttributeSet::GetMaxStaminaAttribute(),
          PawnData->DefaultMaxStamina);
      ASC->SetNumericAttributeBase(UDamageAttributeSet::GetDamageAttribute(),
                                   PawnData->DefaultDamage);
      ASC->SetNumericAttributeBase(
          UDamageAttributeSet::GetKnockbackImpulseAttribute(),
          PawnData->DefaultKnockbackImpulse);
      ASC->SetNumericAttributeBase(
          UDamageAttributeSet::GetLaunchImpulseAttribute(),
          PawnData->DefaultLaunchImpulse);
    }
  }
}

void ACombatEnemy::HandleMovementSpeedChanged(
    const FOnAttributeChangeData &Data) {
  // Update the enemy's movement speed based on the GAS attribute
  if (GetCharacterMovement()) {
    GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
  }
}
