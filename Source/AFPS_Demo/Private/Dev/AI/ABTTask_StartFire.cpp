#include "Dev/AI/ABTTask_StartFire.h"

#include "AIController.h"

#include "Weapons/AWeaponContainerComponent.h"

EBTNodeResult::Type UABTTask_StartFire::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Log, TEXT("BT Start fire"));

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

		WeaponComp->OnTriggerHeldInput();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
