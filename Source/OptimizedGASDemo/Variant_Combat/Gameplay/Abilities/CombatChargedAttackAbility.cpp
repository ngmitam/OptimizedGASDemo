// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatChargedAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatCharacter.h"
#include "CombatEnemy.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagsManager.h"

UCombatChargedAttackAbility::UCombatChargedAttackAbility() {
  // Set up ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.ChargedAttack")));
  SetAssetTags(AssetTags);

  // Block other attacks while charging
  CancelAbilitiesWithTag.AddTag(
      FGameplayTag::RequestGameplayTag(FName("Ability.Attack")));

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
  ACombatCharacter *CombatChar = GetCombatCharacterFromActorInfo();
  ACombatEnemy *CombatEnemy = GetCombatEnemyFromActorInfo();

  if (CombatChar) {
    bIsPlayerControlled = true;
    ChargedAttackMontage = CombatChar->GetChargedAttackMontage();
    ChargeLoopSection = CombatChar->GetChargeLoopSection();
    ChargeAttackSection = CombatChar->GetChargeAttackSection();
  } else if (CombatEnemy) {
    bIsPlayerControlled = false;
    ChargedAttackMontage = CombatEnemy->GetChargedAttackMontage();
    ChargeLoopSection = CombatEnemy->GetChargeLoopSection();
    ChargeAttackSection = CombatEnemy->GetChargeAttackSection();
    // For AI, set random target loops
    TargetChargeLoops = FMath::RandRange(MinChargeLoops, MaxChargeLoops);
  }

  CurrentChargeLoop = 0;

  // Play montage
  if (ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f);

      // Set end delegate to notify character
      if (ACombatEnemy *Enemy = Cast<ACombatEnemy>(ActorInfo->OwnerActor)) {
        FOnMontageEnded Delegate;
        Delegate.BindUObject(Enemy, &ACombatEnemy::NotifyAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(Delegate, ChargedAttackMontage);
      } else if (ACombatCharacter *Char =
                     Cast<ACombatCharacter>(ActorInfo->OwnerActor)) {
        FOnMontageEnded Delegate;
        Delegate.BindUObject(Char, &ACombatCharacter::NotifyAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(Delegate, ChargedAttackMontage);
      }
    }
  }

  // Listen for events
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo()) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(
            FGameplayTag::RequestGameplayTag(FName("Event.ChargedAttack.Loop")))
        .AddUObject(this,
                    &UCombatChargedAttackAbility::HandleChargedAttackLoop);
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.ChargedAttack.Release")))
        .AddUObject(this,
                    &UCombatChargedAttackAbility::HandleChargedAttackRelease);
  }
}

void UCombatChargedAttackAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {

  // Stop montage
  if (ChargedAttackMontage) {
    if (UAnimInstance *AnimInstance = ActorInfo->GetAnimInstance()) {
      AnimInstance->Montage_Stop(0.0f, ChargedAttackMontage);
    }
  }

  // Remove event listeners
  if (UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo()) {
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(
            FGameplayTag::RequestGameplayTag(FName("Event.ChargedAttack.Loop")))
        .RemoveAll(this);
    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(FGameplayTag::RequestGameplayTag(
            FName("Event.ChargedAttack.Release")))
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
  // Release attack
  if (UAnimInstance *AnimInstance = GetCurrentActorInfo()->GetAnimInstance()) {
    AnimInstance->Montage_JumpToSection(ChargeAttackSection,
                                        ChargedAttackMontage);
  }
}
