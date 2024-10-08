#include "AHealthPickup.h"

#include "AHealthComponent.h"

void AAHealthPickup::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
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

	UAHealthComponent* HealthComp = Cast<UAHealthComponent>(OtherActor->GetComponentByClass(UAHealthComponent::StaticClass()));

	if (HealthComp)
	{
		if (HealthComp->AddHealth(HealAmount, bCanOverheal, this)) // Heal was successful
		{
			StartCooldown();
		}
	}
}
