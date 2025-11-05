// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CombatAttackEventData.generated.h"

/**
 * Data structure for attack event information
 */
UCLASS()
class UCombatAttackEventData : public UObject {
  GENERATED_BODY()

public:
  UCombatAttackEventData() {}

  /** The bone name used as the source of the attack damage */
  UPROPERTY()
  FName DamageSourceBone;
};
