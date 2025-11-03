// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AttackEventData.generated.h"

/**
 * Data structure for attack event information
 */
UCLASS()
class UAttackEventData : public UObject {
  GENERATED_BODY()

public:
  UAttackEventData() {}

  /** The bone name used as the source of the attack damage */
  UPROPERTY()
  FName DamageSourceBone;
};
