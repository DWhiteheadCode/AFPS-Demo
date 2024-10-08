#include "AStackPickup.h"

#include "AStackComponent.h"

void AAStackPickup::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor overlapped health pickup, but pickup was on cooldown"));
		return;
	}

	if (!OtherActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Nullptr actor overlapped with health pickup"));
		return;
	}

	UAStackComponent* StackComp = Cast<UAStackComponent>(OtherActor->GetComponentByClass(UAStackComponent::StaticClass()));

	if (StackComp)
	{
		bool bHealedHealth = StackComp->AddHealth(HealthAmount, bCanOverhealHealth, this);
		bool bHealedArmour = StackComp->AddArmour(ArmourAmount, bCanOverhealArmour, this);

		if (bHealedHealth || bHealedArmour)
		{
			StartCooldown();
		}		
	}
}
