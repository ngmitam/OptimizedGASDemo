// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MovementAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                           \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet for Movement
 */
UCLASS()
class UMovementAttributeSet : public UAttributeSet {
  GENERATED_BODY()

public:
  UMovementAttributeSet();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  // Movement Speed attribute
  UPROPERTY(BlueprintReadOnly, Category = "Movement",
            ReplicatedUsing = OnRep_MovementSpeed)
  FGameplayAttributeData MovementSpeed;
  ATTRIBUTE_ACCESSORS(UMovementAttributeSet, MovementSpeed)

  // Max Movement Speed attribute
  UPROPERTY(BlueprintReadOnly, Category = "Movement",
            ReplicatedUsing = OnRep_MaxMovementSpeed)
  FGameplayAttributeData MaxMovementSpeed;
  ATTRIBUTE_ACCESSORS(UMovementAttributeSet, MaxMovementSpeed)

protected:
  UFUNCTION()
  virtual void
  OnRep_MovementSpeed(const FGameplayAttributeData &OldMovementSpeed);

  UFUNCTION()
  virtual void
  OnRep_MaxMovementSpeed(const FGameplayAttributeData &OldMaxMovementSpeed);
};
