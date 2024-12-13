#include "APlayerController.h"

#include "AStackComponent.h"

void AAPlayerController::SetPawn(APawn* NewPawn)
{
	Super::SetPawn(NewPawn);

	if (!IsLocalController())
	{
		return;
	}

	if (NewPawn)
	{
		UAStackComponent* StackComp = Cast<UAStackComponent>(NewPawn->GetComponentByClass(UAStackComponent::StaticClass()));

		if (StackComp)
		{
			StackComp->OnStackChanged.AddDynamic(this, &AAPlayerController::OnPawnStackChanged);
		}
	}
}

void AAPlayerController::OnPawnStackChanged(UAStackComponent* OwningComp, AActor* InstigatorActor, int NewHealth, int DeltaHealth, int NewArmour, int DeltaArmour, int TotalDelta)
{
	if (TotalDelta < 0.f)
	{
		OnDamageTaken.Broadcast(this, OwningComp, InstigatorActor, -TotalDelta);
	}
}

void AAPlayerController::ClientOnDamageDealt_Implementation(UAStackComponent* TargetStackComp, const int DamageAmount)
{
	OnDamageDealt.Broadcast(this, TargetStackComp, DamageAmount);
}
