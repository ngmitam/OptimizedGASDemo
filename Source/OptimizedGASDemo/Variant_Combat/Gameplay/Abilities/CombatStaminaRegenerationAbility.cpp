// Copyright Nguyen Minh Tam. All Rights Reserved.

#include "CombatStaminaRegenerationAbility.h"
#include "AbilitySystemComponent.h"
#include "CombatBase.h"
#include "Attributes/StaminaAttributeSet.h"
#include "Effects/CombatStaminaRegenerationGameplayEffect.h"
#include "Effects/CombatStaminaStunGameplayEffect.h"
#include "GameFramework/PlayerState.h"

UCombatStaminaRegenerationAbility::UCombatStaminaRegenerationAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

  // Set ability tags
  FGameplayTagContainer AssetTags;
  AssetTags.AddTag(FGameplayTag::RequestGameplayTag(
      FName("Ability.Type.StaminaRegeneration")));
  AssetTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.StaminaRegenerating")));
  SetAssetTags(AssetTags);

  // Granted tags
  ActivationOwnedTags.AddTag(
      FGameplayTag::RequestGameplayTag(FName("State.StaminaRegenerating")));

  // Set default effect class
  RegenerationEffectClass =
      UCombatStaminaRegenerationGameplayEffect::StaticClass();
  StunEffectClass = UCombatStaminaStunGameplayEffect::StaticClass();
}

void UCombatStaminaRegenerationAbility::OnGiveAbility(
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilitySpec &Spec) {
  Super::OnGiveAbility(ActorInfo, Spec);

  // Try to activate the ability immediately upon granting (server-only for
  // multiplayer safety)
  if (ActorInfo && GetAbilitySystemComponent(ActorInfo) &&
      ActorInfo->AvatarActor.Get() &&
      ActorInfo->AvatarActor.Get()->HasAuthority()) {
    GetAbilitySystemComponent(ActorInfo)->TryActivateAbility(Spec.Handle);
  }
}

void UCombatStaminaRegenerationAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {

  UAbilitySystemComponent *ASC = GetAbilitySystemComponent(ActorInfo);
  if (!ASC) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    return;
  }

  // Check if this is a player character (has PlayerState as owner)
  bool bIsPlayer = Cast<APlayerState>(ActorInfo->OwnerActor.Get()) != nullptr;

  if (bIsPlayer) {
    // For players, apply regeneration effect immediately and continuously
    ApplyRegenerationEffect();
  } else {
    // For enemies, listen to stamina changes
    OnStaminaChangedDelegate.BindUObject(
        this, &UCombatStaminaRegenerationAbility::OnStaminaChanged);
    StaminaChangedDelegateHandle =
        ASC->GetGameplayAttributeValueChangeDelegate(
               UStaminaAttributeSet::GetStaminaAttribute())
            .Add(OnStaminaChangedDelegate);
  }
}

void UCombatStaminaRegenerationAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {

  RemoveRegenerationEffect();

  // Clear timers
  if (RefillTimerHandle.IsValid()) {
    GetWorld()->GetTimerManager().ClearTimer(RefillTimerHandle);
  }

  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);
}

void UCombatStaminaRegenerationAbility::OnDepleted() {
  // Apply stun effect to immobilize the enemy
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());
  if (ASC && StunEffectClass) {
    FGameplayEffectSpecHandle SpecHandle =
        ASC->MakeOutgoingSpec(StunEffectClass, 1.0f, ASC->MakeEffectContext());
    if (SpecHandle.IsValid()) {
      StunEffectHandle =
          ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
    }
  }

  // Start refill timer for enemies
  GetWorld()->GetTimerManager().SetTimer(
      RefillTimerHandle, this,
      &UCombatStaminaRegenerationAbility::RefillStamina, EnemyRefillDelay,
      false);
}

void UCombatStaminaRegenerationAbility::OnRestored() {
  // Remove stun effect
  if (StunEffectHandle.IsValid()) {
    GetAbilitySystemComponent(GetCurrentActorInfo())
        ->RemoveActiveGameplayEffect(StunEffectHandle);
    StunEffectHandle.Invalidate();
  }

  // Clear timer if stamina was restored before timer
  if (RefillTimerHandle.IsValid()) {
    GetWorld()->GetTimerManager().ClearTimer(RefillTimerHandle);
  }
}

void UCombatStaminaRegenerationAbility::OnStaminaChanged(
    const FOnAttributeChangeData &Data) {
  // Only handle enemy logic (players use continuous regeneration)
  bool bIsPlayer =
      Cast<APlayerState>(
          GetAbilitySystemComponent(GetCurrentActorInfo())->GetOwnerActor()) !=
      nullptr;
  if (bIsPlayer)
    return;

  // Enemy logic: refill after depletion and stun when depleted
  if (Data.NewValue <= 0.0f) {
    // Stamina depleted, apply stun effect
    OnDepleted();
  } else if (Data.NewValue > 0.0f) {
    // Stamina restored, remove stun effect
    OnRestored();
  }
}

void UCombatStaminaRegenerationAbility::ApplyRegenerationEffect() {
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());
  if (!ASC || !RegenerationEffectClass)
    return;

  FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
      RegenerationEffectClass, 1.0f, ASC->MakeEffectContext());
  if (SpecHandle.IsValid()) {
    // Set the regeneration amount per period (rate * period)
    SpecHandle.Data.Get()->SetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("Data.StaminaRegenRate")),
        PlayerRegenerationRate * 0.1f);

    ActiveEffectHandle =
        ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
  }
}

void UCombatStaminaRegenerationAbility::RemoveRegenerationEffect() {
  if (ActiveEffectHandle.IsValid()) {
    UAbilitySystemComponent *ASC =
        GetAbilitySystemComponent(GetCurrentActorInfo());
    if (ASC) {
      ASC->RemoveActiveGameplayEffect(ActiveEffectHandle);
    }
    ActiveEffectHandle.Invalidate();
  }
}

void UCombatStaminaRegenerationAbility::RefillStamina() {
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());
  if (!ASC)
    return;

  // Set stamina to max
  float MaxStamina =
      ASC->GetNumericAttribute(UStaminaAttributeSet::GetMaxStaminaAttribute());
  ASC->SetNumericAttributeBase(UStaminaAttributeSet::GetStaminaAttribute(),
                               MaxStamina);
}
