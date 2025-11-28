// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatLifeBar.h"
#include "CombatPlayerController.h"
#include "CombatPlayerState.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/DamageAttributeSet.h"
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

  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

  // Configure character movement
  GetCharacterMovement()->MaxWalkSpeed =
      600.0f; // Increased for better combat mobility
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
    PawnData->GrantedAbilities.Add(UCombatLockToggleAbility::StaticClass());

    // Note: Abilities are now configured via AbilitySets in PawnData
  }

  // create the lock system component
  LockSystemComponent = CreateDefaultSubobject<UCombatLockSystemComponent>(
      TEXT("LockSystemComponent"));

  // set the player tag
  Tags.Add(FName("Player"));
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
  // Send gameplay event to toggle lock
  SendGameplayEvent(
      FGameplayTag::RequestGameplayTag(FName("Event.Lock.Toggle")));
}

void ACombatCharacter::UpdateCameraLock(float DeltaTime) {
  if (LockSystemComponent && LockSystemComponent->HasLockedTarget()) {
    AActor *Target = LockSystemComponent->GetLockedTarget();
    if (Target && CameraBoom) {
      FVector CameraLocation = CameraBoom->GetComponentLocation();
      FVector TargetLocation = Target->GetActorLocation();
      FRotator DesiredRotation = (TargetLocation - CameraLocation).Rotation();

      // Dynamic camera distance based on combat state
      float TargetDistance =
          bIsAttacking ? CombatCameraDistance : ExplorationCameraDistance;
      CameraBoom->TargetArmLength = FMath::FInterpTo(
          CameraBoom->TargetArmLength, TargetDistance, DeltaTime, 5.0f);

      // Smooth interpolation to target rotation
      if (!bCameraLocked) {
        // First time locking, snap to avoid delay
        CameraBoom->SetWorldRotation(DesiredRotation);
        bCameraLocked = true;
      } else {
        // Interpolate smoothly
        FRotator CurrentRotation = CameraBoom->GetComponentRotation();
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation, DesiredRotation, DeltaTime, CameraLockInterpSpeed);
        CameraBoom->SetWorldRotation(NewRotation);
      }

      // Make the character face the target for movement
      if (GetController()) {
        FRotator CharacterRotation =
            (TargetLocation - GetActorLocation()).Rotation();
        CharacterRotation.Pitch = 0.0f; // Keep the character level
        GetController()->SetControlRotation(CharacterRotation);
      }
    }

    // Check if target is still valid, if not, unlock
    if (!LockSystemComponent->IsTargetStillValid()) {
      LockSystemComponent->UnlockTarget();
      bCameraLocked = false;
      // Re-enable pawn control rotation
      if (CameraBoom) {
        CameraBoom->bUsePawnControlRotation = true;
      }
      // Remove locked state tag
      if (AbilitySystemComponent) {
        AbilitySystemComponent->RemoveLooseGameplayTag(
            FGameplayTag::RequestGameplayTag(FName("State.Locked")));
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

  // Execute any buffered attack
  ExecuteBufferedAttack();
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

  // Check if we can attack immediately
  if (!bIsAttacking) {
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
  } else {
    // Buffer the attack input
    bAttackBuffered = true;
    BufferedAttackType = 1; // Combo
    GetWorld()->GetTimerManager().SetTimer(AttackBufferTimer, this,
                                           &ACombatCharacter::ClearAttackBuffer,
                                           AttackBufferTime, false);
  }
}

void ACombatCharacter::DoComboAttackEnd() {
  // stub
}

void ACombatCharacter::DoChargedAttackStart() {
  // Store current velocity for momentum preservation
  StoredVelocity = GetVelocity();

  // raise the charging attack flag
  bIsChargingAttack = true;

  if (!bIsAttacking) {
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
  } else {
    // Buffer the attack input
    bAttackBuffered = true;
    BufferedAttackType = 2; // Charged
    GetWorld()->GetTimerManager().SetTimer(AttackBufferTimer, this,
                                           &ACombatCharacter::ClearAttackBuffer,
                                           AttackBufferTime, false);
  }
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
  UCombatHealthComponent *HealthComp = GetHealthComponent();
  if (HealthComp && !HealthComp->IsDead()) {
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

    // Lock Target
    EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered,
                                       this, &ACombatCharacter::LockPressed);
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

void ACombatCharacter::LockOntoTarget() {
  if (LockSystemComponent) {
    LockSystemComponent->LockOntoTarget();
  }
}

void ACombatCharacter::UnlockTarget() {
  if (LockSystemComponent) {
    LockSystemComponent->UnlockTarget();
  }
}

bool ACombatCharacter::HasLockedTarget() const {
  return LockSystemComponent && LockSystemComponent->HasLockedTarget();
}

void ACombatCharacter::ClearAttackBuffer() {
  bAttackBuffered = false;
  BufferedAttackType = 0;
}

void ACombatCharacter::ExecuteBufferedAttack() {
  if (bAttackBuffered && !bIsAttacking) {
    if (BufferedAttackType == 1) {
      // Execute buffered combo attack
      SendGameplayEvent(
          FGameplayTag::RequestGameplayTag(FName("Event.Attack.Combo.Start")));
    } else if (BufferedAttackType == 2) {
      // Execute buffered charged attack
      SendGameplayEvent(FGameplayTag::RequestGameplayTag(
          FName("Event.Attack.Charged.Start")));
    }
    ClearAttackBuffer();
  }
}
