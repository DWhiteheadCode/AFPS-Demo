#include "Dev/AI/AShootingDummyAiController.h"

void AAShootingDummyAiController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);
}