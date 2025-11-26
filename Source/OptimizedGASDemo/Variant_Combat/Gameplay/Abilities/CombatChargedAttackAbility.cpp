// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatChargedAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatBase.h"
#include "CombatCharacter.h"
#include "AI/CombatEnemy.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagsManager.h"

UCombatChargedAttackAbility::UCombatChargedAttackAbility() {
  // Set up ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack.Charged")));
  SetAssetTags(AssetTags);

  // Add state tag while active
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.Attacking")));

  // Block other attack abilities while charging
  BlockAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Attack")));

  // Cancel this ability if death occurs
  CancelAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Type.Death")));

  // Add trigger to activate on charged start event
  FAbilityTriggerData StartTriggerData;
  StartTriggerData.TriggerTag =
      FGameplayTag::RequestGameplayTag(FName("Event.Attack.Charged.Start"));
  StartTriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
  AbilityTriggers.Add(StartTriggerData);

  // Set default montage sections
  ChargeLoopSection = FName("ChargeLoop");
  ChargeAttackSection = FName("ChargeAttack");
}

void UCombatChargedAttackAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Determine if player or AI and get montage
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();

  if (CombatBase) {
    if (Cast<ACombatCharacter>(CombatBase)) {
      bIsPlayerControlled = true;
    } else if (ACombatEnemy *CombatEnemy = Cast<ACombatEnemy>(CombatBase)) {
      bIsPlayerControlled = false;
      // For AI, set random target loops
      TargetChargeLoops = FMath::RandRange(CombatEnemy->GetMinChargeLoops(),
                                           CombatEnemy->GetMaxChargeLoops());
    }

    ChargedAttackMontage = CombatBase->GetChargedAttackMontage();
    ChargeLoopSection = CombatBase->GetChargeLoopSection();
    ChargeAttackSection = CombatBase->GetChargeAttackSection();

    // Set attacking flag
    CombatBase->SetIsAttacking(true);

    // Enable root motion on skeletal mesh for attack movement
    if (USkeletalMeshComponent *Mesh = CombatBase->GetMesh()) {
      Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
      // Root motion will automatically drive movement during attacks
    }

    // Apply momentum preservation if character has stored velocity
    if (ACombatCharacter *CombatChar = Cast<ACombatCharacter>(CombatBase)) {
      if (!CombatChar->GetStoredVelocity().IsNearlyZero()) {
        CombatBase->GetCharacterMovement()->AddImpulse(
            CombatChar->GetStoredVelocity() *
                CombatChar->GetAttackMomentumMultiplier(),
            true);
      }
    }

    // Notify enemies of attack via ability
    if (UAbilitySystemComponent *ASC =
            GetAbilitySystemComponentFromActorInfo()) {
      FGameplayEventData EventData;
      EventData.Instigator = TObjectPtr<AActor>(CombatBase);
      EventData.Target = TObjectPtr<AActor>(CombatBase);
      ASC->HandleGameplayEvent(
          FGameplayTag::RequestGameplayTag(FName("Event.Notify.Enemies")),
          &EventData);
    }
  }

  CurrentChargeLoop = 0;

  // Play montage
  if (ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f);

      // Set end delegate
      FOnMontageEnded MontageEndDelegate;
      MontageEndDelegate.BindUObject(
          this, &UCombatChargedAttackAbility::OnMontageEnded);
      AnimInstance->Montage_SetEndDelegate(MontageEndDelegate,
                                           ChargedAttackMontage);
    }
  } else {
    // No montage available
  }

  // Listen for events
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo()) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Loop")))
        .AddUObject(this,
                    &UCombatChargedAttackAbility::HandleChargedAttackLoop);
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Release")))
        .AddUObject(this,
                    &UCombatChargedAttackAbility::HandleChargedAttackRelease);
  }
}

void UCombatChargedAttackAbility::OnMontageEnded(UAnimMontage *Montage,
                                                 bool bInterrupted) {
  // Call the character's attack montage ended function
  if (AActor *Avatar = GetCurrentActorInfo()->AvatarActor.Get()) {
    if (ACombatBase *CombatBase = Cast<ACombatBase>(Avatar)) {
      CombatBase->AttackMontageEnded(Montage, bInterrupted);
    }
  }

  EndAbility(CurrentSpecHandle, GetCurrentActorInfo(),
             GetCurrentActivationInfo(), false, bInterrupted);
}

void UCombatChargedAttackAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {
  // Reset attacking flag
  ACombatBase *CombatBase = GetCombatBaseFromActorInfo();
  if (CombatBase) {
    CombatBase->SetIsAttacking(false);
  }

  // Stop montage only if cancelled
  if (bWasCancelled && ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Stop(0.0f, ChargedAttackMontage);
    }
  }

  // Remove montage end delegate only if cancelled
  if (bWasCancelled && ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      FOnMontageEnded EmptyDelegate;
      AnimInstance->Montage_SetEndDelegate(EmptyDelegate, ChargedAttackMontage);
    }
  }

  // Remove event listeners
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo()) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Loop")))
        .RemoveAll(this);
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.Attack.Charged.Release")))
        .RemoveAll(this);
  }

  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);
}

void UCombatChargedAttackAbility::HandleChargedAttackLoop(
    const FGameplayEventData *EventData) {
  if (bIsPlayerControlled) {
    // For player, always loop if still holding
    if (UAnimInstance *AnimInstance =
            GetCurrentActorInfo()->GetAnimInstance()) {
      AnimInstance->Montage_JumpToSection(ChargeLoopSection,
                                          ChargedAttackMontage);
    }
  } else {
    // For AI, check if we need to loop more
    CurrentChargeLoop++;
    if (CurrentChargeLoop < TargetChargeLoops) {
      if (UAnimInstance *AnimInstance =
              GetCurrentActorInfo()->GetAnimInstance()) {
        AnimInstance->Montage_JumpToSection(ChargeLoopSection,
                                            ChargedAttackMontage);
      }
    } else {
      // Go to attack
      if (UAnimInstance *AnimInstance =
              GetCurrentActorInfo()->GetAnimInstance()) {
        AnimInstance->Montage_JumpToSection(ChargeAttackSection,
                                            ChargedAttackMontage);
      }
    }
  }
}

void UCombatChargedAttackAbility::HandleChargedAttackRelease(
    const FGameplayEventData *EventData) {
  if (!bIsPlayerControlled && CurrentChargeLoop < TargetChargeLoops) {
    // For AI, if not enough loops, continue looping
    CurrentChargeLoop++;
    if (UAnimInstance *AnimInstance =
            GetCurrentActorInfo()->GetAnimInstance()) {
      AnimInstance->Montage_JumpToSection(ChargeLoopSection,
                                          ChargedAttackMontage);
    }
  } else {
    // Release attack
    if (UAnimInstance *AnimInstance =
            GetCurrentActorInfo()->GetAnimInstance()) {
      AnimInstance->Montage_JumpToSection(ChargeAttackSection,
                                          ChargedAttackMontage);
    }
  }
}
