// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CombatAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                           \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet for Combat variant
 */
UCLASS()
class UCombatAttributeSet : public UAttributeSet {
  GENERATED_BODY()

public:
  UCombatAttributeSet();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  // Health
  UPROPERTY(BlueprintReadOnly, Category = "Health",
            ReplicatedUsing = OnRep_Health)
  FGameplayAttributeData Health;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Health)

  UPROPERTY(BlueprintReadOnly, Category = "Health",
            ReplicatedUsing = OnRep_MaxHealth)
  FGameplayAttributeData MaxHealth;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MaxHealth)

  // Damage
  UPROPERTY(BlueprintReadOnly, Category = "Damage",
            ReplicatedUsing = OnRep_Damage)
  FGameplayAttributeData Damage;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Damage)

  // Knockback
  UPROPERTY(BlueprintReadOnly, Category = "Damage",
            ReplicatedUsing = OnRep_KnockbackImpulse)
  FGameplayAttributeData KnockbackImpulse;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, KnockbackImpulse)

  // Launch
  UPROPERTY(BlueprintReadOnly, Category = "Damage",
            ReplicatedUsing = OnRep_LaunchImpulse)
  FGameplayAttributeData LaunchImpulse;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, LaunchImpulse)

  // Trace
  UPROPERTY(BlueprintReadOnly, Category = "Trace",
            ReplicatedUsing = OnRep_TraceDistance)
  FGameplayAttributeData TraceDistance;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, TraceDistance)

  UPROPERTY(BlueprintReadOnly, Category = "Trace",
            ReplicatedUsing = OnRep_TraceRadius)
  FGameplayAttributeData TraceRadius;
  ATTRIBUTE_ACCESSORS(UCombatAttributeSet, TraceRadius)

protected:
  UFUNCTION()
  virtual void OnRep_Health(const FGameplayAttributeData &OldHealth);

  UFUNCTION()
  virtual void OnRep_MaxHealth(const FGameplayAttributeData &OldMaxHealth);

  UFUNCTION()
  virtual void OnRep_Damage(const FGameplayAttributeData &OldDamage);

  UFUNCTION()
  virtual void
  OnRep_KnockbackImpulse(const FGameplayAttributeData &OldKnockbackImpulse);

  UFUNCTION()
  virtual void
  OnRep_LaunchImpulse(const FGameplayAttributeData &OldLaunchImpulse);

  UFUNCTION()
  virtual void
  OnRep_TraceDistance(const FGameplayAttributeData &OldTraceDistance);

  UFUNCTION()
  virtual void OnRep_TraceRadius(const FGameplayAttributeData &OldTraceRadius);
};
