// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatLifeBar.h"
#include "CombatPlayerController.h"
#include "CombatPlayerState.h"
#include "Attributes/HealthAttributeSet.h"
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

#if !UE_BUILD_SHIPPING
  // Debug display for stamina used and damage taken
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponent()) {
    float Stamina =
        ASC->GetNumericAttribute(UStaminaAttributeSet::GetStaminaAttribute());
    float MaxStamina = ASC->GetNumericAttribute(
        UStaminaAttributeSet::GetMaxStaminaAttribute());
    float StaminaUsed = ASC->GetNumericAttribute(
        UStaminaAttributeSet::GetStaminaUsedAttribute());
    float Health =
        ASC->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute());
    float MaxHealth =
        ASC->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());

    FString DebugText =
        FString::Printf(TEXT("Player - Stamina: %.1f/%.1f, Stamina Used: %.1f, "
                             "Health: %.1f/%.1f"),
                        Stamina, MaxStamina, StaminaUsed, Health, MaxHealth);

    DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 150),
                    DebugText, nullptr, FColor::Yellow, 0.0f, true, 1.0f);
  }
#endif
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
    // Bind inputs from InputConfig if available
    if (PawnData && PawnData->InputConfig) {
      BindInputsFromConfig(EnhancedInputComponent);
    } else {
      // Fallback to basic movement inputs only
      BindBasicMovementInputs(EnhancedInputComponent);
    }
  }
}

void ACombatCharacter::BindInputsFromConfig(
    UEnhancedInputComponent *EnhancedInputComponent) {
  UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
  if (!ASC) {
    // If no ASC, we can't use abilities anyway, so just bind basic movement
    BindBasicMovementInputs(EnhancedInputComponent);
    return;
  }

  // Bind each input mapping
  for (const FCombatInputMapping &Mapping :
       PawnData->InputConfig->InputMappings) {
    if (!Mapping.InputAction)
      continue;

    // For each trigger in this mapping
    for (const FCombatAbilityTriggerData &Trigger : Mapping.AbilityTriggers) {
      if (Trigger.TriggerEvent == ETriggerEvent::Started) {
        EnhancedInputComponent->BindAction(
            Mapping.InputAction, Trigger.TriggerEvent, this,
            &ACombatCharacter::SendAbilityTrigger, Trigger.TriggerTag);
      } else if (Trigger.TriggerEvent == ETriggerEvent::Completed) {
        EnhancedInputComponent->BindAction(
            Mapping.InputAction, Trigger.TriggerEvent, this,
            &ACombatCharacter::SendAbilityTrigger, Trigger.TriggerTag);
      }
      // Add other trigger events as needed
    }
  }

  // Also bind movement and look inputs
  BindBasicMovementInputs(EnhancedInputComponent);
}

void ACombatCharacter::BindBasicMovementInputs(
    UEnhancedInputComponent *EnhancedInputComponent) {
  // Moving
  if (MoveAction) {
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                       this, &ACombatCharacter::Move);
  }

  // Looking
  if (LookAction) {
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                       this, &ACombatCharacter::Look);
  }
  if (MouseLookAction) {
    EnhancedInputComponent->BindAction(MouseLookAction,
                                       ETriggerEvent::Triggered, this,
                                       &ACombatCharacter::Look);
  }
}

void ACombatCharacter::SendAbilityTrigger(FGameplayTag TriggerTag) {
  // Send gameplay event with the trigger tag
  SendGameplayEvent(TriggerTag);
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
    PS->ResetAttributesToDefault();
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
