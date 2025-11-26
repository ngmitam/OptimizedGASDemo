// Copyright Nguyen Minh Tam. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OptimizedGASDemoGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AOptimizedGASDemoGameMode : public AGameModeBase {
  GENERATED_BODY()

public:
  /** Constructor */
  AOptimizedGASDemoGameMode();
};
