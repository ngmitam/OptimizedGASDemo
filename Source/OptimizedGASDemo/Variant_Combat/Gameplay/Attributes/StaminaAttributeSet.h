// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StaminaAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS_STAMINA(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet for Stamina
 */
UCLASS()
class UStaminaAttributeSet : public UAttributeSet {
  GENERATED_BODY()

public:
  UStaminaAttributeSet();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  // Stamina
  UPROPERTY(BlueprintReadOnly, Category = "Stamina",
            ReplicatedUsing = OnRep_Stamina)
  FGameplayAttributeData Stamina;
  ATTRIBUTE_ACCESSORS_STAMINA(UStaminaAttributeSet, Stamina)

  UPROPERTY(BlueprintReadOnly, Category = "Stamina",
            ReplicatedUsing = OnRep_MaxStamina)
  FGameplayAttributeData MaxStamina;
  ATTRIBUTE_ACCESSORS_STAMINA(UStaminaAttributeSet, MaxStamina)

  // Stamina used for next attack (temporary for damage scaling)
  UPROPERTY(BlueprintReadOnly, Category = "Stamina",
            ReplicatedUsing = OnRep_StaminaUsed)
  FGameplayAttributeData StaminaUsed;
  ATTRIBUTE_ACCESSORS_STAMINA(UStaminaAttributeSet, StaminaUsed)

protected:
  UFUNCTION()
  virtual void OnRep_Stamina(const FGameplayAttributeData &OldStamina);

  UFUNCTION()
  virtual void OnRep_MaxStamina(const FGameplayAttributeData &OldMaxStamina);

  UFUNCTION()
  virtual void OnRep_StaminaUsed(const FGameplayAttributeData &OldStaminaUsed);

  virtual void PreAttributeChange(const FGameplayAttribute &Attribute,
                                  float &NewValue) override;
};
