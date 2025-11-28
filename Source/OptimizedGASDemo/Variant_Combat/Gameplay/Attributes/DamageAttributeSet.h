// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DamageAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                           \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet for Damage
 */
UCLASS()
class UDamageAttributeSet : public UAttributeSet {
  GENERATED_BODY()

public:
  UDamageAttributeSet();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  // Damage
  UPROPERTY(BlueprintReadOnly, Category = "Damage",
            ReplicatedUsing = OnRep_Damage)
  FGameplayAttributeData Damage;
  ATTRIBUTE_ACCESSORS(UDamageAttributeSet, Damage)

  // Knockback
  UPROPERTY(BlueprintReadOnly, Category = "Damage",
            ReplicatedUsing = OnRep_KnockbackImpulse)
  FGameplayAttributeData KnockbackImpulse;
  ATTRIBUTE_ACCESSORS(UDamageAttributeSet, KnockbackImpulse)

  // Launch
  UPROPERTY(BlueprintReadOnly, Category = "Damage",
            ReplicatedUsing = OnRep_LaunchImpulse)
  FGameplayAttributeData LaunchImpulse;
  ATTRIBUTE_ACCESSORS(UDamageAttributeSet, LaunchImpulse)

protected:
  UFUNCTION()
  virtual void OnRep_Damage(const FGameplayAttributeData &OldDamage);

  UFUNCTION()
  virtual void
  OnRep_KnockbackImpulse(const FGameplayAttributeData &OldKnockbackImpulse);

  UFUNCTION()
  virtual void
  OnRep_LaunchImpulse(const FGameplayAttributeData &OldLaunchImpulse);
};
