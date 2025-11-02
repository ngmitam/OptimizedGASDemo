// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatLifeBar.h"
#include "CombatPlayerController.h"
#include "CombatPlayerState.h"
#include "Gameplay/CombatAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"
#include "Gameplay/CombatReceiveDamageAbility.h"
#include "Gameplay/CombatDeathAbility.h"
#include "Gameplay/CombatAttackTraceAbility.h"
#include "Gameplay/CombatDamageGameplayEffect.h"

ACombatCharacter::ACombatCharacter() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatCharacter::AttackMontageEnded);

  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

  // Configure character movement
  GetCharacterMovement()->MaxWalkSpeed = 400.0f;

  // create the camera boom
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);

  CameraBoom->TargetArmLength = DefaultCameraDistance;
  CameraBoom->bUsePawnControlRotation = true;
  CameraBoom->bEnableCameraLag = true;
  CameraBoom->bEnableCameraRotationLag = true;

  // create the orbiting camera
  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
  FollowCamera->bUsePawnControlRotation = false;

  // create the life bar widget component
  LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
  LifeBar->SetupAttachment(RootComponent);

  // create the health component
  HealthComponent =
      CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

  // create the ability system component
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));

  // create the attribute set
  AttributeSet =
      CreateDefaultSubobject<UCombatAttributeSet>(TEXT("AttributeSet"));

  // Create default pawn data with abilities
  PawnData = CreateDefaultSubobject<UCombatPawnData>(TEXT("PawnData"));
  if (PawnData) {
    // Set default attributes
    PawnData->DefaultHealth = 100.0f;
    PawnData->DefaultMaxHealth = 100.0f;
    PawnData->DefaultDamage = 5.0f;
    PawnData->DefaultKnockbackImpulse = 500.0f;
    PawnData->DefaultLaunchImpulse = 300.0f;
    PawnData->DefaultTraceDistance = 75.0f;
    PawnData->DefaultTraceRadius = 75.0f;

    // Add granted abilities
    PawnData->GrantedAbilities.Add(UCombatReceiveDamageAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatDeathAbility::StaticClass());
    PawnData->GrantedAbilities.Add(UCombatAttackTraceAbility::StaticClass());

    // Add granted effects (none for now, but structure is ready)
    // PawnData->GrantedEffects.Add(SomeEffectClass::StaticClass());
  }

  // set the player tag
  Tags.Add(FName("Player"));
}

UAbilitySystemComponent *ACombatCharacter::GetAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

void ACombatCharacter::Move(const FInputActionValue &Value) {
  // input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  // route the input
  DoMove(MovementVector.X, MovementVector.Y);
}

void ACombatCharacter::Look(const FInputActionValue &Value) {
  FVector2D LookAxisVector = Value.Get<FVector2D>();

  // route the input
  DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACombatCharacter::ComboAttackPressed() {
  // route the input
  DoComboAttackStart();
}

void ACombatCharacter::ChargedAttackPressed() {
  // route the input
  DoChargedAttackStart();
}

void ACombatCharacter::ChargedAttackReleased() {
  // route the input
  DoChargedAttackEnd();
}

void ACombatCharacter::ToggleCamera() {
  // call the BP hook
  BP_ToggleCamera();
}

void ACombatCharacter::DoMove(float Right, float Forward) {
  if (GetController() != nullptr) {
    // find out which way is forward
    const FRotator Rotation = GetController()->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    // get forward vector
    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // get right vector
    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // add movement
    AddMovementInput(ForwardDirection, Forward);
    AddMovementInput(RightDirection, Right);
  }
}

void ACombatCharacter::DoLook(float Yaw, float Pitch) {
  if (GetController() != nullptr) {
    // add yaw and pitch input to controller
    AddControllerYawInput(Yaw);
    AddControllerPitchInput(Pitch);
  }
}

void ACombatCharacter::DoComboAttackStart() {
  // are we already playing an attack animation?
  if (bIsAttacking) {
    // cache the input time so we can check it later
    CachedAttackInputTime = GetWorld()->GetTimeSeconds();
    return; // Don't start a new attack while already attacking
  }

  // perform a combo attack
  ComboAttack();
}

void ACombatCharacter::DoComboAttackEnd() {
  // stub
}

void ACombatCharacter::DoChargedAttackStart() {
  // raise the charging attack flag
  bIsChargingAttack = true;

  if (bIsAttacking) {
    // cache the input time so we can check it later
    CachedAttackInputTime = GetWorld()->GetTimeSeconds();

    return;
  }

  ChargedAttack();
}

void ACombatCharacter::DoChargedAttackEnd() {
  // lower the charging attack flag
  bIsChargingAttack = false;

  // if we've done the charge loop at least once, release the charged attack
  // right away
  if (bHasLoopedChargedAttack) {
    CheckChargedAttack();
  }
}

void ACombatCharacter::ResetHP() {
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent()) {
    float MaxHealth = HealthComponent->GetMaxHealth();
    ASC->SetNumericAttributeBase(UCombatAttributeSet::GetHealthAttribute(),
                                 MaxHealth);
  }

  // update the life bar
  LifeBarWidget->SetLifePercentage(1.0f);
}

void ACombatCharacter::ComboAttack() {
  // raise the attacking flag
  bIsAttacking = true;

  // reset the combo count
  ComboCount = 0;

  // notify enemies they are about to be attacked
  NotifyEnemiesOfAttack();

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

void ACombatCharacter::ChargedAttack() {
  // raise the attacking flag
  bIsAttacking = true;

  // reset the charge loop flag
  bHasLoopedChargedAttack = false;

  // notify enemies they are about to be attacked
  NotifyEnemiesOfAttack();

  // play the charged attack montage
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

void ACombatCharacter::AttackMontageEnded(UAnimMontage *Montage,
                                          bool bInterrupted) {
  // reset the attacking flag
  bIsAttacking = false;

  // check if we have a non-stale cached input
  if (GetWorld()->GetTimeSeconds() - CachedAttackInputTime <=
      AttackInputCacheTimeTolerance) {
    // are we holding the charged attack button?
    if (bIsChargingAttack) {
      // do a charged attack
      ChargedAttack();
    } else {
      // do a regular attack
      ComboAttack();
    }
  }
}

void ACombatCharacter::DoAttackTrace(FName DamageSourceBone) {
  // Send gameplay event to activate attack trace ability
  if (AbilitySystemComponent) {
    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.Target = this;

    AbilitySystemComponent->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Attack.Start")),
        &EventData);
  }
}

void ACombatCharacter::CheckCombo() {
  // are we playing a non-charge attack animation?
  if (bIsAttacking && !bIsChargingAttack) {
    // is the last attack input not stale?
    if (GetWorld()->GetTimeSeconds() - CachedAttackInputTime <=
        ComboInputCacheTimeTolerance) {
      // consume the attack input so we don't accidentally trigger it twice
      CachedAttackInputTime = 0.0f;

      // increase the combo counter
      ++ComboCount;

      // do we still have a combo section to play?
      if (ComboCount < ComboSectionNames.Num()) {
        // notify enemies they are about to be attacked
        NotifyEnemiesOfAttack();

        // jump to the next combo section
        if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
          AnimInstance->Montage_JumpToSection(ComboSectionNames[ComboCount],
                                              ComboAttackMontage);
        }
      }
    }
  }
}

void ACombatCharacter::CheckChargedAttack() {
  // raise the looped charged attack flag
  bHasLoopedChargedAttack = true;

  // jump to either the loop or the attack section depending on whether we're
  // still holding the charge button
  if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
    AnimInstance->Montage_JumpToSection(bIsChargingAttack ? ChargeLoopSection
                                                          : ChargeAttackSection,
                                        ChargedAttackMontage);
  }
}

void ACombatCharacter::NotifyEnemiesOfAttack() {
  // sweep for objects in front of the character to be hit by the attack
  TArray<FHitResult> OutHits;

  // start at the actor location, sweep forward
  const FVector TraceStart = GetActorLocation();
  const FVector TraceEnd =
      TraceStart + (GetActorForwardVector() * DangerTraceDistance);

  // check for pawn object types only
  FCollisionObjectQueryParams ObjectParams;
  ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

  // use a sphere shape for the sweep
  FCollisionShape CollisionShape;
  CollisionShape.SetSphere(DangerTraceRadius);

  // ignore self
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(this);

  if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd,
                                         FQuat::Identity, ObjectParams,
                                         CollisionShape, QueryParams)) {
    // iterate over each object hit
    for (const FHitResult &CurrentHit : OutHits) {
      // check if we've hit a damageable actor
      ICombatDamageable *Damageable =
          Cast<ICombatDamageable>(CurrentHit.GetActor());

      if (Damageable) {
        // notify the enemy
        Damageable->NotifyDanger(GetActorLocation(), this);
      }
    }
  }
}

void ACombatCharacter::ApplyDamage(float Damage, AActor *DamageCauser,
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
  } else {
    // Fallback for objects without ASC: call ReceivedDamage for effects
    ReceivedDamage(Damage, DamageLocation, -DamageImpulse.GetSafeNormal());
  }
}

void ACombatCharacter::HandleDeath() {
  // Send gameplay event to activate death ability
  if (AbilitySystemComponent) {
    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.Target = this;

    AbilitySystemComponent->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Death")), &EventData);
  } else {
    // disable movement while we're dead
    GetCharacterMovement()->DisableMovement();

    // schedule respawning
    GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this,
                                           &ACombatCharacter::RespawnCharacter,
                                           RespawnTime, false);
  }

  // enable full ragdoll physics
  GetMesh()->SetSimulatePhysics(true);

  // hide the life bar
  LifeBar->SetHiddenInGame(true);

  // pull back the camera
  GetCameraBoom()->TargetArmLength = DeathCameraDistance;
}

void ACombatCharacter::ApplyHealing(float Healing, AActor *Healer) {
  // stub
}

void ACombatCharacter::Landed(const FHitResult &Hit) {
  Super::Landed(Hit);

  // is the character still alive?
  if (HealthComponent && !HealthComponent->IsDead()) {
    // disable ragdoll physics
    GetMesh()->SetPhysicsBlendWeight(0.0f);
  }
}

void ACombatCharacter::BeginPlay() {
  Super::BeginPlay();

  // get the life bar from the widget component
  LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
  check(LifeBarWidget);

  // initialize the camera
  GetCameraBoom()->TargetArmLength = DefaultCameraDistance;

  // save the relative transform for the mesh so we can reset the ragdoll later
  MeshStartingTransform = GetMesh()->GetRelativeTransform();

  // set the life bar color
  LifeBarWidget->SetBarColor(LifeBarColor);

  // reset HP to maximum
  ResetHP();

  // Initialize GAS
  if (AbilitySystemComponent) {
    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    // Initialize attributes
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
    }
  }

  // Initialize health component
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent()) {
    HealthComponent->InitializeWithAbilitySystem(ASC);
  }

  // Bind to health component delegates for UI updates
  HealthComponent->OnHealthChanged.AddUObject(
      this, &ACombatCharacter::OnHealthComponentChanged);
  HealthComponent->OnMaxHealthChanged.AddUObject(
      this, &ACombatCharacter::OnMaxHealthComponentChanged);
}

void ACombatCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  // clear the respawn timer
  GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void ACombatCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Set up action bindings
  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    // Moving
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                       this, &ACombatCharacter::Move);

    // Looking
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                       this, &ACombatCharacter::Look);
    EnhancedInputComponent->BindAction(MouseLookAction,
                                       ETriggerEvent::Triggered, this,
                                       &ACombatCharacter::Look);

    // Combo Attack
    EnhancedInputComponent->BindAction(ComboAttackAction,
                                       ETriggerEvent::Started, this,
                                       &ACombatCharacter::ComboAttackPressed);

    // Charged Attack
    EnhancedInputComponent->BindAction(ChargedAttackAction,
                                       ETriggerEvent::Started, this,
                                       &ACombatCharacter::ChargedAttackPressed);
    EnhancedInputComponent->BindAction(
        ChargedAttackAction, ETriggerEvent::Completed, this,
        &ACombatCharacter::ChargedAttackReleased);

    // Camera Side Toggle
    EnhancedInputComponent->BindAction(ToggleCameraAction,
                                       ETriggerEvent::Triggered, this,
                                       &ACombatCharacter::ToggleCamera);
  }
}

void ACombatCharacter::NotifyControllerChanged() {
  Super::NotifyControllerChanged();

  // update the respawn transform on the Player Controller
  if (ACombatPlayerController *PC =
          Cast<ACombatPlayerController>(GetController())) {
    PC->SetRespawnTransform(GetActorTransform());
  }
}

void ACombatCharacter::OnHealthComponentChanged(float NewHealth) {
  if (LifeBarWidget) {
    float MaxHealth = HealthComponent->GetMaxHealth();
    LifeBarWidget->SetLifePercentage(NewHealth / MaxHealth);
  }
}

void ACombatCharacter::OnMaxHealthComponentChanged(float NewMaxHealth) {
  if (LifeBarWidget) {
    float CurrentHealth = HealthComponent->GetHealth();
    LifeBarWidget->SetLifePercentage(CurrentHealth / NewMaxHealth);
  }
}

void ACombatCharacter::NotifyDanger(const FVector &DangerLocation,
                                    AActor *DangerSource) {
  // Implement if needed, e.g., play animation or something
}

void ACombatCharacter::RespawnCharacter() {
  // Destroy the current character
  Destroy();

  // The PlayerController will handle spawning a new one
}
