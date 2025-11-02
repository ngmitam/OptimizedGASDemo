// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "AI/CombatEnemy.h"

ACombatAIController::ACombatAIController() {
  // create the StateTree AI Component
  StateTreeAI =
      CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
  check(StateTreeAI);

  // ensure we start the StateTree when we possess the pawn
  bStartAILogicOnPossess = true;

  // ensure we're attached to the possessed character.
  // this is necessary for EnvQueries to work correctly
  bAttachToPawn = true;
}

void ACombatAIController::OnPossess(APawn *InPawn) {
  Super::OnPossess(InPawn);

  // Ensure StateTree has the correct context after possession
  if (StateTreeAI && InPawn) {
    if (InPawn->IsA(ACombatEnemy::StaticClass())) {
      // Stop the StateTree if it's running to reset context
      if (StateTreeAI->IsRunning()) {
        StateTreeAI->StopLogic(TEXT("Repossessing pawn"));
      }

      // Restart the StateTree logic - it should automatically pick up the
      // possessed pawn as context
      StateTreeAI->StartLogic();
    }
  }
}
