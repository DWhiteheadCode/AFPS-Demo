#include "Dev/AI/ABTTask_StopFire.h"

#include "AIController.h"

#include "Weapons/AWeaponContainerComponent.h"

EBTNodeResult::Type UABTTask_StopFire::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (MyController)
	{
		APawn* MyPawn = MyController->GetPawn();
		if (!MyPawn)
		{
			UE_LOG(LogTemp, Error, TEXT("StartFire BTTask called, but MyController didn't possess a Pawn"));
			return EBTNodeResult::Failed;
		}

		UAWeaponContainerComponent* WeaponComp = Cast<UAWeaponContainerComponent>(MyPawn->GetComponentByClass(UAWeaponContainerComponent::StaticClass()));
		if (!WeaponComp)
		{
			UE_LOG(LogTemp, Error, TEXT("StartFire BTTask called, but MyPawn didn't have a WeaponContainerComponent"));
			return EBTNodeResult::Failed;
		}

		WeaponComp->OnTriggerReleasedInput();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
