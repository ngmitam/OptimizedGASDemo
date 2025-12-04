// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                           \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet for Health
 */
UCLASS()
class UHealthAttributeSet : public UAttributeSet {
  GENERATED_BODY()

public:
  UHealthAttributeSet();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  virtual void PreAttributeChange(const FGameplayAttribute &Attribute,
                                  float &NewValue) override;

  virtual void PostAttributeChange(const FGameplayAttribute &Attribute,
                                   float OldValue, float NewValue) override;

  // Health
  UPROPERTY(BlueprintReadOnly, Category = "Health",
            ReplicatedUsing = OnRep_Health)
  FGameplayAttributeData Health;
  ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health)

  UPROPERTY(BlueprintReadOnly, Category = "Health",
            ReplicatedUsing = OnRep_MaxHealth)
  FGameplayAttributeData MaxHealth;
  ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth)

protected:
  UFUNCTION()
  virtual void OnRep_Health(const FGameplayAttributeData &OldHealth);

  UFUNCTION()
  virtual void OnRep_MaxHealth(const FGameplayAttributeData &OldMaxHealth);
};
