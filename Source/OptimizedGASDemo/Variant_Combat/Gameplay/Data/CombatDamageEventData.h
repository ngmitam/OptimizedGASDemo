// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CombatDamageEventData.generated.h"

UCLASS()
class UCombatDamageEventData : public UObject {
  GENERATED_BODY()
public:
  FVector Location;
  FVector Impulse;
};
