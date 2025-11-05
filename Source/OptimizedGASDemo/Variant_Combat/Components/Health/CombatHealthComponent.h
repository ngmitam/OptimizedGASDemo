// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "AttributeSet.h"
#include "CombatHealthComponent.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

// Delegates for health changes
DECLARE_MULTICAST_DELEGATE_OneParam(FHealth_HealthChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FHealth_MaxHealthChanged, float);

/**
 * Component that manages health for characters using the Ability System.
 * Follows Lyra's HealthComponent pattern.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCombatHealthComponent : public UGameFrameworkComponent {
  GENERATED_BODY()

public:
  UCombatHealthComponent(const FObjectInitializer &ObjectInitializer);

  //~UGameFrameworkComponent interface
  virtual void OnRegister() override;
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  //~End of UGameFrameworkComponent interface

  // Initialize the component with an ability system
  void InitializeWithAbilitySystem(UAbilitySystemComponent *InASC);

  // Get current health
  UFUNCTION(BlueprintPure, Category = "Health")
  float GetHealth() const;

  // Get max health
  UFUNCTION(BlueprintPure, Category = "Health")
  float GetMaxHealth() const;

  // Check if the owner is dead
  UFUNCTION(BlueprintPure, Category = "Health")
  bool IsDead() const;

  // Get health as a normalized value (0-1)
  UFUNCTION(BlueprintPure, Category = "Health")
  float GetHealthNormalized() const;

  // Delegates for health changes
  FHealth_HealthChanged OnHealthChanged;
  FHealth_MaxHealthChanged OnMaxHealthChanged;

protected:
  // Handle health attribute changes
  void HandleHealthChanged(const FOnAttributeChangeData &ChangeData);

  // Handle max health attribute changes
  void HandleMaxHealthChanged(const FOnAttributeChangeData &ChangeData);

  // Called when the owner dies
  void HandleDeath();

  // Called when the owner is revived
  void HandleRevive();

protected:
  // Cached ability system component
  UPROPERTY()
  UAbilitySystemComponent *AbilitySystemComponent;

  // Cached health attribute
  FGameplayAttribute HealthAttribute;

  // Cached max health attribute
  FGameplayAttribute MaxHealthAttribute;

  // Delegate handles for attribute changes
  FDelegateHandle HealthChangedDelegateHandle;
  FDelegateHandle MaxHealthChangedDelegateHandle;
};
