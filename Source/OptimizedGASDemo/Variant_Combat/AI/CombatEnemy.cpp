// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatEnemy.h"
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
#include "CombatAttributeSet.h"
#include "GameplayTagsManager.h"

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

  // create the ability system component
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));
  AbilitySystemComponent->SetIsReplicated(
      false); // Enemies don't need replication

  // create the attribute set
  AttributeSet =
      CreateDefaultSubobject<UCombatAttributeSet>(TEXT("AttributeSet"));

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

  // raise the attacking flag
  bIsAttacking = true;

  // choose how many times we're going to attack
  TargetComboCount = FMath::RandRange(1, ComboSectionNames.Num() - 1);

  // reset the attack counter
  CurrentComboAttack = 0;

  // play the attack montage
  if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
    const float MontageLength = AnimInstance->Montage_Play(
        ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f,
        true);

    // subscribe to montage completed and interrupted events
    if (MontageLength > 0.0f) {
      // set the end delegate for the montage
      AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded,
                                           ComboAttackMontage);
    }
  }
}

void ACombatEnemy::DoAIChargedAttack() {
  // ignore if we're already playing an attack animation
  if (bIsAttacking) {
    return;
  }

  // raise the attacking flag
  bIsAttacking = true;

  // choose how many loops are we going to charge for
  TargetChargeLoops = FMath::RandRange(MinChargeLoops, MaxChargeLoops);

  // reset the charge loop counter
  CurrentChargeLoop = 0;

  // play the attack montage
  if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
    const float MontageLength = AnimInstance->Montage_Play(
        ChargedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f,
        true);

    // subscribe to montage completed and interrupted events
    if (MontageLength > 0.0f) {
      // set the end delegate for the montage
      AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded,
                                           ChargedAttackMontage);
    }
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
  float Damage = MeleeDamage;
  float Knockback = MeleeKnockbackImpulse;
  float Launch = MeleeLaunchImpulse;

  if (AbilitySystemComponent) {
    Damage = AbilitySystemComponent->GetNumericAttribute(
        UCombatAttributeSet::GetDamageAttribute());
    Knockback = AbilitySystemComponent->GetNumericAttribute(
        UCombatAttributeSet::GetKnockbackImpulseAttribute());
    Launch = AbilitySystemComponent->GetNumericAttribute(
        UCombatAttributeSet::GetLaunchImpulseAttribute());
  }

  // sweep for objects in front of the character to be hit by the attack
  TArray<FHitResult> OutHits;

  // start at the provided socket location, sweep forward
  const FVector TraceStart = GetMesh()->GetSocketLocation(DamageSourceBone);
  const FVector TraceEnd =
      TraceStart + (GetActorForwardVector() * MeleeTraceDistance);

  // enemies only affect Pawn collision objects; they don't knock back boxes
  FCollisionObjectQueryParams ObjectParams;
  ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

  // use a sphere shape for the sweep
  FCollisionShape CollisionShape;
  CollisionShape.SetSphere(MeleeTraceRadius);

  // ignore self
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(this);

  if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd,
                                         FQuat::Identity, ObjectParams,
                                         CollisionShape, QueryParams)) {
    // iterate over each object hit
    for (const FHitResult &CurrentHit : OutHits) {
      /** does the actor have the player tag? */
      if (CurrentHit.GetActor()->ActorHasTag(FName("Player"))) {
        // check if the actor is damageable
        ICombatDamageable *Damageable =
            Cast<ICombatDamageable>(CurrentHit.GetActor());

        if (Damageable) {
          // knock upwards and away from the impact normal
          const FVector Impulse = (CurrentHit.ImpactNormal * -Knockback) +
                                  (FVector::UpVector * Launch);

          // pass the damage event to the actor
          Damageable->ApplyDamage(Damage, this, CurrentHit.ImpactPoint,
                                  Impulse);
        }
      }
    }
  }
}

void ACombatEnemy::CheckCombo() {
  // increase the combo counter
  ++CurrentComboAttack;

  // do we still have attacks to play in this string?
  if (CurrentComboAttack < TargetComboCount) {
    // jump to the next attack section
    if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
      AnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboAttack],
                                          ComboAttackMontage);
    }
  }
}

void ACombatEnemy::CheckChargedAttack() {
  // increase the charge loop counter
  ++CurrentChargeLoop;

  // jump to either the loop or attack section of the montage depending on
  // whether we hit the loop target
  if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
    AnimInstance->Montage_JumpToSection(CurrentChargeLoop >= TargetChargeLoops
                                            ? ChargeAttackSection
                                            : ChargeLoopSection,
                                        ChargedAttackMontage);
  }
}

void ACombatEnemy::ApplyDamage(float Damage, AActor *DamageCauser,
                               const FVector &DamageLocation,
                               const FVector &DamageImpulse) {

  // pass the damage event to the actor
  FDamageEvent DamageEvent;
  const float ActualDamage =
      TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);

  // only process knockback and effects if we received nonzero damage
  if (ActualDamage > 0.0f) {
    // apply the knockback impulse
    GetCharacterMovement()->AddImpulse(DamageImpulse, true);

    // is the character ragdolling?
    if (GetMesh()->IsSimulatingPhysics()) {
      // apply an impulse to the ragdoll
      GetMesh()->AddImpulseAtLocation(DamageImpulse * GetMesh()->GetMass(),
                                      DamageLocation);
    }

    // stop the attack montages to interrupt the attack
    if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
      AnimInstance->Montage_Stop(0.1f, ComboAttackMontage);
      AnimInstance->Montage_Stop(0.1f, ChargedAttackMontage);
    }

    // pass control to BP to play effects, etc.
    ReceivedDamage(ActualDamage, DamageLocation, DamageImpulse.GetSafeNormal());
  }
}

void ACombatEnemy::HandleDeath() {
  // hide the life bar
  LifeBar->SetHiddenInGame(true);

  // disable the collision capsule to avoid being hit again while dead
  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // disable character movement
  GetCharacterMovement()->DisableMovement();

  // enable full ragdoll physics
  GetMesh()->SetSimulatePhysics(true);

  // call the died delegate to notify any subscribers
  OnEnemyDied.Broadcast();

  // set up the death timer
  GetWorld()->GetTimerManager().SetTimer(
      DeathTimer, this, &ACombatEnemy::RemoveFromLevel, DeathRemovalTime);
}

void ACombatEnemy::ApplyHealing(float Healing, AActor *Healer) {
  // stub
}

void ACombatEnemy::NotifyDanger(const FVector &DangerLocation,
                                AActor *DangerSource) {
  // ensure we're being attacked by the player
  if (DangerSource && DangerSource->ActorHasTag(FName("Player"))) {
    // save the danger location and game time
    LastDangerLocation = DangerLocation;
    LastDangerTime = GetWorld()->GetTimeSeconds();
  }
}

void ACombatEnemy::RemoveFromLevel() {
  // destroy this actor
  Destroy();
}

float ACombatEnemy::TakeDamage(float Damage,
                               struct FDamageEvent const &DamageEvent,
                               AController *EventInstigator,
                               AActor *DamageCauser) {
  // only process damage if the character is still alive
  if (CurrentHP <= 0.0f) {
    return 0.0f;
  }

  // reduce the current HP
  CurrentHP -= Damage;

  // sync with GAS
  if (AbilitySystemComponent) {
    AbilitySystemComponent->SetNumericAttributeBase(
        UCombatAttributeSet::GetHealthAttribute(), CurrentHP);
  }

  // have we run out of HP?
  if (CurrentHP <= 0.0f) {
    // die
    HandleDeath();
  } else {
    // update the life bar
    LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);

    // enable partial ragdoll physics, but keep the pelvis vertical
    GetMesh()->SetPhysicsBlendWeight(0.5f);
    GetMesh()->SetBodySimulatePhysics(PelvisBoneName, false);
  }

  // return the received damage amount
  return Damage;
}

void ACombatEnemy::Landed(const FHitResult &Hit) {
  Super::Landed(Hit);

  // is the character still alive?
  if (CurrentHP >= 0.0f) {
    // disable ragdoll physics
    GetMesh()->SetPhysicsBlendWeight(0.0f);
  }

  // call the landed Delegate for StateTree
  OnEnemyLanded.ExecuteIfBound();
}

void ACombatEnemy::BeginPlay() {
  // reset HP to maximum
  CurrentHP = MaxHP;

  // we top the HP before BeginPlay so StateTree picks it up at the right value
  Super::BeginPlay();

  // init ability system
  if (AbilitySystemComponent) {
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
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

    // Bind delegates to sync HP variables with GAS
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(
            UCombatAttributeSet::GetHealthAttribute())
        .AddUObject(this, &ACombatEnemy::OnHealthChanged);
    AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(
            UCombatAttributeSet::GetMaxHealthAttribute())
        .AddUObject(this, &ACombatEnemy::OnMaxHealthChanged);
  }

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

void ACombatEnemy::OnHealthChanged(const FOnAttributeChangeData &Data) {
  CurrentHP = Data.NewValue;
  if (LifeBarWidget) {
    float MaxHealth = AbilitySystemComponent
                          ? AbilitySystemComponent->GetNumericAttribute(
                                UCombatAttributeSet::GetMaxHealthAttribute())
                          : MaxHP;
    LifeBarWidget->SetLifePercentage(CurrentHP / MaxHealth);
  }
}

void ACombatEnemy::OnMaxHealthChanged(const FOnAttributeChangeData &Data) {
  MaxHP = Data.NewValue;
  if (LifeBarWidget) {
    LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);
  }
}
