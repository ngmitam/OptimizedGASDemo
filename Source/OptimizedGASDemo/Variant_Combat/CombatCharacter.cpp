// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatLifeBar.h"
#include "CombatPlayerController.h"
#include "CombatPlayerState.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Attributes/MovementAttributeSet.h"
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
#include "Abilities/CombatReceiveDamageAbility.h"
#include "Abilities/CombatDeathAbility.h"
#include "Abilities/CombatTraceAttackAbility.h"
#include "Abilities/CombatChargedAttackAbility.h"
#include "Abilities/CombatComboAttackAbility.h"
#include "Abilities/CombatNotifyEnemiesAbility.h"
#include "Abilities/CombatLockToggleAbility.h"
#include "Effects/CombatDamageGameplayEffect.h"
#include "Data/CombatAttackEventData.h"
ACombatCharacter::ACombatCharacter() {
  PrimaryActorTick.bCanEverTick = true;

  // bind the attack montage ended delegate
  OnAttackMontageEnded.BindUObject(this, &ACombatCharacter::AttackMontageEnded);

  // Configure character movement
  OriginalMaxWalkSpeed = 600.0f;
  GetCharacterMovement()->MaxWalkSpeed =
      OriginalMaxWalkSpeed; // Increased for better combat mobility
  GetCharacterMovement()->MaxAcceleration = 2000.0f; // Faster acceleration
  GetCharacterMovement()->GroundFriction = 8.0f;     // Better control
  GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
  GetCharacterMovement()->bUseSeparateBrakingFriction = true;

  // Enable root motion for attack animations
  GetCharacterMovement()->bUseControllerDesiredRotation = true;
  GetCharacterMovement()->bOrientRotationToMovement = false;

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

  // create the lock system component
  LockSystemComponent = CreateDefaultSubobject<UCombatLockSystemComponent>(
      TEXT("LockSystemComponent"));

  // Add lock toggle ability for player
  if (PawnData) {
    PawnData->GrantedAbilities.Add(UCombatLockToggleAbility::StaticClass());
    PawnData->DefaultDamage = 10.0f;
  }

  // set the player tag
  Tags.Add(FName("Player"));
}

UAbilitySystemComponent *ACombatCharacter::GetAbilitySystemComponent() const {
  // Per Lyra standard, player characters get GAS from PlayerState
  if (ACombatPlayerState *PS = Cast<ACombatPlayerState>(GetPlayerState())) {
    return PS->GetAbilitySystemComponent();
  }
  // Fallback to base implementation for enemies or other cases
  return Super::GetAbilitySystemComponent();
}

void ACombatCharacter::Move(const FInputActionValue &Value) {
  // input is a Vector2D
  FVector2D RawInput = Value.Get<FVector2D>();

  // Apply deadzone
  if (RawInput.Size() < InputDeadzone) {
    RawInput = FVector2D::ZeroVector;
  }

  // Smooth input
  SmoothedInput = FMath::Lerp(SmoothedInput, RawInput, InputSmoothingFactor);

  // route the input
  DoMove(SmoothedInput.X, SmoothedInput.Y);
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

void ACombatCharacter::LockPressed() {
  // Prevent rapid lock presses
  float CurrentTime = GetWorld()->GetTimeSeconds();
  if (CurrentTime - LastLockPressTime < 0.2f) {
    return;
  }
  LastLockPressTime = CurrentTime;

  // Client prediction
  if (!HasAuthority()) {
    if (LockSystemComponent) {
      LockSystemComponent->Client_PredictToggleLock();
    }
  }

  // Send gameplay event to toggle lock
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Lock.Toggle")));
}

void ACombatCharacter::UpdateCameraLock(float DeltaTime) {
  if (HasLockedTarget()) {
    AActor *Target = LockSystemComponent->GetLockedTarget();
    if (Target) {
      if (!CameraBoom) {
        return;
      }
      FVector CameraLocation = CameraBoom->GetComponentLocation();
      FVector TargetLocation = Target->GetActorLocation();
      FRotator DesiredRotation = (TargetLocation - CameraLocation).Rotation();

      CameraBoom->TargetArmLength = FMath::FInterpTo(
          CameraBoom->TargetArmLength, CombatCameraDistance, DeltaTime, 5.0f);

      // Smooth interpolation to target rotation
      FRotator CurrentRelativeRotation = CameraBoom->GetRelativeRotation();
      FRotator TargetRelativeRotation = DesiredRotation - GetActorRotation();
      TargetRelativeRotation = TargetRelativeRotation.GetNormalized();

      if (!bCameraLocked) {
        // First time locking, snap to avoid delay
        CameraBoom->bUsePawnControlRotation =
            false; // Disable pawn control to manually control rotation
        CameraBoom->SetRelativeRotation(TargetRelativeRotation);
        bCameraLocked = true;
      } else {
        // Interpolate smoothly
        FRotator NewRotation =
            FMath::RInterpTo(CurrentRelativeRotation, TargetRelativeRotation,
                             DeltaTime, CameraLockInterpSpeed);
        CameraBoom->SetRelativeRotation(NewRotation);
      }

      // Make the character face the target for movement
      if (GetController()) {
        FRotator CharacterRotation =
            (TargetLocation - GetActorLocation()).Rotation();
        CharacterRotation.Pitch = 0.0f; // Keep the character level
        GetController()->SetControlRotation(CharacterRotation);
      }
    }
  } else {
    bCameraLocked = false;
    // Reset to exploration distance when not locked
    if (CameraBoom) {
      CameraBoom->TargetArmLength =
          FMath::FInterpTo(CameraBoom->TargetArmLength,
                           ExplorationCameraDistance, DeltaTime, 5.0f);
    }
  }
}

void ACombatCharacter::AttackMontageEnded(UAnimMontage *Montage,
                                          bool bInterrupted) {
  // Call parent to reset attacking flag
  Super::AttackMontageEnded(Montage, bInterrupted);
}

void ACombatCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  UpdateCameraLock(DeltaTime);
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
  // Store current velocity for momentum preservation
  StoredVelocity = GetVelocity();

  // Cache the input time for combo system
  SetCachedComboAttackInputTime(GetWorld()->GetTimeSeconds());

  // Send gameplay event to activate combo attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Start")));

  // Add camera shake for feedback
  if (AttackCameraShake) {
    if (APlayerController *PC = Cast<APlayerController>(GetController())) {
      if (PC->PlayerCameraManager) {
        PC->PlayerCameraManager->StartCameraShake(AttackCameraShake);
      }
    }
  }
}

void ACombatCharacter::DoComboAttackEnd() {
  // stub
}

void ACombatCharacter::DoChargedAttackStart() {
  // Store current velocity for momentum preservation
  StoredVelocity = GetVelocity();

  // Send gameplay event to activate charged attack ability
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Start")));

  // Add camera shake for feedback
  if (AttackCameraShake) {
    if (APlayerController *PC = Cast<APlayerController>(GetController())) {
      if (PC->PlayerCameraManager) {
        PC->PlayerCameraManager->StartCameraShake(AttackCameraShake);
      }
    }
  }
}

void ACombatCharacter::DoChargedAttackEnd() {
  // Send release event
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (ASC) {
    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.Target = this;

    ASC->HandleGameplayEvent(
        FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Release")),
        &EventData);
  }
}

void ACombatCharacter::Landed(const FHitResult &Hit) {
  Super::Landed(Hit);

  // is the character still alive?
  UCombatHealthComponent *HealthComp = GetHealthComponent();
  if (HealthComp && !HealthComp->IsDead()) {
    // disable ragdoll physics
    GetMesh()->SetPhysicsBlendWeight(0.0f);
  }
}

void ACombatCharacter::BeginPlay() {
  Super::BeginPlay();

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

    // Lock Target
    EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered,
                                       this, &ACombatCharacter::LockPressed);
  }
}

void ACombatCharacter::NotifyControllerChanged() {
  Super::NotifyControllerChanged();

  // Set pawn data on player state for GAS management per Lyra standard
  if (ACombatPlayerState *PS = Cast<ACombatPlayerState>(GetPlayerState())) {
    PS->SetPawnData(PawnData);
  }

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

bool ACombatCharacter::HasLockedTarget() const {
  return LockSystemComponent && LockSystemComponent->HasLockedTarget();
}

void ACombatCharacter::PossessedBy(AController *NewController) {
  Super::PossessedBy(NewController);

  // Set pawn data on player state for GAS management (needed for respawn)
  if (ACombatPlayerState *PS = Cast<ACombatPlayerState>(GetPlayerState())) {
    PS->SetPawnData(PawnData);

    // Re-initialize ability actor info with the new pawn (important for
    // respawn)
    if (UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent()) {
      ASC->InitAbilityActorInfo(PS, this);
    }
  }

  // Initialize health and stamina components with the ability system
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (HealthComponent) {
    HealthComponent->InitializeWithAbilitySystem(ASC);
  }
  if (StaminaComponent) {
    StaminaComponent->InitializeWithAbilitySystem(ASC);
  }

  // Reset attributes to default values on possession (initial spawn or respawn)
  if (ACombatPlayerState *PS = Cast<ACombatPlayerState>(GetPlayerState())) {
    if (ASC) {
      ASC->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(),
                                   PS->GetDefaultMaxHP());
      ASC->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(),
                                   PS->GetDefaultMaxHP());
      ASC->SetNumericAttributeBase(UStaminaAttributeSet::GetStaminaAttribute(),
                                   PS->GetDefaultMaxStamina());
      ASC->SetNumericAttributeBase(
          UStaminaAttributeSet::GetMaxStaminaAttribute(),
          PS->GetDefaultMaxStamina());
    }
  }

  // Initialize movement attributes and bind delegate
  InitializeMovementAttributes(ASC);
}

void ACombatCharacter::HandleMovementSpeedChanged(
    const FOnAttributeChangeData &Data) {
  // Update the character's movement speed based on the GAS attribute
  if (GetCharacterMovement()) {
    GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
  }
}
