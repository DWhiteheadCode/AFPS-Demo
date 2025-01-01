#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AShootingDummyAiController.generated.h"

class UBehaviorTree;

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAShootingDummyAiController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;
};
