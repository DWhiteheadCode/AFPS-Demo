#include "Pickups/APickup_Stack.h"

#include "AStackComponent.h"

void AAPickup_Stack::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("Stack pickup overlap"));

	if (bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor overlapped stack pickup [%s], but pickup was on cooldown"), *GetNameSafe(this));
		return;
	}

	if (!OtherActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Nullptr actor overlapped with stack pickup [%s]"), *GetNameSafe(this));
		return;
	}

	UAStackComponent* StackComp = Cast<UAStackComponent>(OtherActor->GetComponentByClass(UAStackComponent::StaticClass()));

	if (StackComp)
	{
		const bool bHealedHealth = StackComp->AddHealth(HealthAmount, bCanOverhealHealth, this);
		const bool bHealedArmour = StackComp->AddArmour(ArmourAmount, bCanOverhealArmour, this);

		if (bHealedHealth || bHealedArmour)
		{
			if (bRespawns)
			{
				UE_LOG(LogTemp, Log, TEXT("Starting stack pickup cooldown"));
				StartCooldown();
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Consuming stack pickup"));
				bIsOnCooldown = true;
				UpdatePickupState();
				SetLifeSpan(2.f);
			}
		}		
	}
}
