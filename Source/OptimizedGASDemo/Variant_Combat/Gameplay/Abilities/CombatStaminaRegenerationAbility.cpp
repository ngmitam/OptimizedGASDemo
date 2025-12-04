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
    // Apply regeneration effect only for players
    ApplyRegenerationEffect();
  } else {
    // For enemies, listen to stamina changes for stun/refill logic (no
    // auto-regeneration)
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

  // Check if this is a player character (has PlayerState as owner)
  bool bIsPlayer = Cast<APlayerState>(ActorInfo->OwnerActor.Get()) != nullptr;

  if (bIsPlayer) {
    // Only remove regeneration effect for players
    RemoveRegenerationEffect();
  }

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
  // Only handle enemy logic (players use execution-based regeneration)
  bool bIsPlayer =
      Cast<APlayerState>(
          GetAbilitySystemComponent(GetCurrentActorInfo())->GetOwnerActor()) !=
      nullptr;
  if (bIsPlayer)
    return;

  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());

  // Enemy logic: if stamina > 0 but not enough for attack (10), reset to 0
  if (Data.NewValue > 0.0f && Data.NewValue < 10.0f) {
    ASC->SetNumericAttributeBase(UStaminaAttributeSet::GetStaminaAttribute(),
                                 0.0f);
  }

  // Enemy logic: refill after depletion and stun when depleted
  if (Data.NewValue <= 0.0f) {
    // Stamina depleted, apply stun effect
    OnDepleted();
  } else if (Data.NewValue > 0.0f) {
    // Stamina restored, remove stun effect
    OnRestored();
  }
}

void UCombatStaminaRegenerationAbility::OnTagChanged(const FGameplayTag Tag,
                                                     int32 NewCount) {
  // Only handle enemy logic (players use execution-based regeneration)
  bool bIsPlayer =
      Cast<APlayerState>(
          GetAbilitySystemComponent(GetCurrentActorInfo())->GetOwnerActor()) !=
      nullptr;
  if (!bIsPlayer)
    return;

  // No player logic needed - handled by execution
}

void UCombatStaminaRegenerationAbility::CheckAttackState() {
  // No longer needed - handled by execution
}

void UCombatStaminaRegenerationAbility::ApplyRegenerationEffect() {
  UAbilitySystemComponent *ASC =
      GetAbilitySystemComponent(GetCurrentActorInfo());
  if (!ASC || !RegenerationEffectClass)
    return;

  // Only apply regeneration effect to players (enemies use refill logic)
  bool bIsPlayer = Cast<APlayerState>(ASC->GetOwnerActor()) != nullptr;
  if (!bIsPlayer)
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
