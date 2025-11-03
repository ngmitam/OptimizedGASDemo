// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DamageEventData.generated.h"

UCLASS()
class UDamageEventData : public UObject {
  GENERATED_BODY()
public:
  FVector Location;
  FVector Impulse;
};
