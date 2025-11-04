// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatLifeBar.h"
#include "CombatPlayerController.h"
#include "CombatPlayerState.h"
#include "Gameplay/Attributes/CombatAttributeSet.h"
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
#include "Gameplay/Abilities/CombatReceiveDamageAbility.h"
#include "Gameplay/Abilities/CombatDeathAbility.h"
#include "Gameplay/Abilities/CombatTraceAttackAbility.h"
#include "Gameplay/Abilities/CombatChargedAttackAbility.h"
#include "Gameplay/Abilities/CombatComboAttackAbility.h"
#include "Gameplay/Abilities/CombatNotifyEnemiesAbility.h"
#include "Gameplay/Effects/CombatDamageGameplayEffect.h"
#include "Gameplay/Data/AttackEventData.h"

/** Constructor */
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
  }

  // set the player tag
  Tags.Add(FName("Player"));
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
    CachedComboAttackInputTime = GetWorld()->GetTimeSeconds();
    return; // Don't start a new attack while already attacking
  }

  // Send gameplay event to activate combo attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Start")));
}

void ACombatCharacter::DoComboAttackEnd() {
  // stub
}

void ACombatCharacter::DoChargedAttackStart() {
  // raise the charging attack flag
  bIsChargingAttack = true;

  if (bIsAttacking) {
    // cache the input time so we can check it later
    CachedChargedAttackInputTime = GetWorld()->GetTimeSeconds();
    return;
  }

  // Send gameplay event to activate charged attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Start")));
}

void ACombatCharacter::DoChargedAttackEnd() {
  // lower the charging attack flag
  bIsChargingAttack = false;

  // if we've done the charge loop at least once, release the charged attack
  // right away
  if (bHasLoopedChargedAttack) {
    // Send release event
    if (AbilitySystemComponent) {
      FGameplayEventData EventData;
      EventData.Instigator = this;
      EventData.Target = this;

      AbilitySystemComponent->HandleGameplayEvent(
          FGameplayTag::RequestGameplayTag(
              FName("Event.Attack.Charged.Release")),
          &EventData);
    }
  }
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

  // save the relative transform for the mesh so we can reset the
  // ragdoll later
  MeshStartingTransform = GetMesh()->GetRelativeTransform();

  // set the life bar color
  LifeBarWidget->SetBarColor(LifeBarColor);

  // reset HP to maximum
  ResetHP();
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

void ACombatCharacter::RespawnCharacter() {
  // Destroy the current character
  Destroy();

  // The PlayerController will handle spawning a new one
}
