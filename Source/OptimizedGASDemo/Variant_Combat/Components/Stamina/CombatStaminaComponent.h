// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "AttributeSet.h"
#include "CombatStaminaComponent.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

// Delegates for stamina changes
DECLARE_MULTICAST_DELEGATE_OneParam(FStamina_StaminaChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FStamina_MaxStaminaChanged, float);

/**
 * Component that manages stamina for characters using the Ability System.
 * Mirrors Lyra's HealthComponent pattern.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCombatStaminaComponent : public UGameFrameworkComponent {
  GENERATED_BODY()

public:
  UCombatStaminaComponent(const FObjectInitializer &ObjectInitializer);

  //~UGameFrameworkComponent interface
  virtual void OnRegister() override;
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  //~End of UGameFrameworkComponent interface

  // Initialize the component with an ability system
  void InitializeWithAbilitySystem(UAbilitySystemComponent *ASC);

  // Get current stamina
  UFUNCTION(BlueprintPure, Category = "Stamina")
  float GetStamina() const;

  // Get max stamina
  UFUNCTION(BlueprintPure, Category = "Stamina")
  float GetMaxStamina() const;

  // Check if stamina is depleted
  UFUNCTION(BlueprintPure, Category = "Stamina")
  bool IsDepleted() const;

  // Get stamina as a normalized value (0-1)
  UFUNCTION(BlueprintPure, Category = "Stamina")
  float GetStaminaNormalized() const;

  // Delegates for stamina changes
  FStamina_StaminaChanged OnStaminaChanged;
  FStamina_MaxStaminaChanged OnMaxStaminaChanged;

protected:
  // Handle stamina attribute changes
  void HandleStaminaChanged(const FOnAttributeChangeData &ChangeData);

  // Handle max stamina attribute changes
  void HandleMaxStaminaChanged(const FOnAttributeChangeData &ChangeData);

  // Called when stamina is fully depleted
  void HandleDepleted();

  // Called when stamina is restored from zero
  void HandleRestored();

protected:
  // Cached ability system component
  UPROPERTY()
  UAbilitySystemComponent *AbilitySystemComponent;

  // Cached stamina attribute
  FGameplayAttribute StaminaAttribute;

  // Cached max stamina attribute
  FGameplayAttribute MaxStaminaAttribute;

  // Delegate handles for attribute changes
  FDelegateHandle StaminaChangedDelegateHandle;
  FDelegateHandle MaxStaminaChangedDelegateHandle;
};
