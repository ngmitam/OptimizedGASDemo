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
#include "GameplayTagsManager.h"
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatNotifyEnemiesAbility.h"
#include "Abilities/CombatLockableAbility.h"
#include "Effects/CombatDamageGameplayEffect.h"
#include "Data/CombatAttackEventData.h"

/** Constructor */
ACombatEnemy::ACombatEnemy() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatEnemy::AttackMontageEnded);

  // create the life bar widget component
  LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
  LifeBar->SetupAttachment(RootComponent);

  // create the health component
  HealthComponent =
      CreateDefaultSubobject<UCombatHealthComponent>(TEXT("HealthComponent"));

  // create the ability system component
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));

  // create the attribute sets
  HealthAttributeSet =
      CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
  DamageAttributeSet =
      CreateDefaultSubobject<UDamageAttributeSet>(TEXT("DamageAttributeSet"));

  // Add attribute sets to ASC
  // Note: Attribute sets are now added from AbilitySets in PawnData or as
  // fallback
  // AbilitySystemComponent->AddAttributeSetSubobject(HealthAttributeSet);
  // AbilitySystemComponent->AddAttributeSetSubobject(DamageAttributeSet);

  // Create default pawn data with abilities
  PawnData = CreateDefaultSubobject<UCombatPawnData>(TEXT("PawnData"));
  if (PawnData) {
    // Set attribute sets
    PawnData->AttributeSets.Add(UHealthAttributeSet::StaticClass());
    PawnData->AttributeSets.Add(UDamageAttributeSet::StaticClass());

    // Set default attributes
    PawnData->DefaultHealth = MaxHP;
    PawnData->DefaultMaxHealth = MaxHP;
    PawnData->DefaultDamage = MeleeDamage;
    PawnData->DefaultKnockbackImpulse = MeleeKnockbackImpulse;
    PawnData->DefaultLaunchImpulse = MeleeLaunchImpulse;

    // Set default abilities (fallback when no AbilitySets are configured)
    PawnData->GrantedAbilities.Add(UCombatReceiveDamageAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatDeathAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatTraceAttackAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatChargedAttackAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatComboAttackAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatLockableAbility::StaticClass());

    // Note: Abilities are now configured via AbilitySets in PawnData
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

void ACombatEnemy::DoAIComboAttack() {
  // ignore if we're already playing an attack animation
  if (bIsAttacking) {
    return;
  }

  // Send gameplay event to activate combo attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Start")));
}

void ACombatEnemy::DoAIChargedAttack() {
  // ignore if we're already playing an attack animation
  if (bIsAttacking) {
    return;
  }

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

  // enable full ragdoll physics
  GetMesh()->SetSimulatePhysics(true);

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

  Super::BeginPlay();

  // get the life bar widget from the widget comp
  LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
  check(LifeBarWidget);

  // fill the life bar
  LifeBarWidget->SetLifePercentage(1.0f);
}

void ACombatEnemy::EndPlay(EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  // clear the death timer
  GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}
