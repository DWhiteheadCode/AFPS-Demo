#include "Pickups/APickup_Stack.h"

#include "AStackComponent.h"

void AAPickup_Stack::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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
		bool bHealedHealth = StackComp->AddHealth(HealthAmount, bCanOverhealHealth, this);
		bool bHealedArmour = StackComp->AddArmour(ArmourAmount, bCanOverhealArmour, this);

		if (bHealedHealth || bHealedArmour)
		{
			if (bRespawns)
			{
				StartCooldown();
			}
			else
			{
				bIsOnCooldown = true;
				UpdatePickupState();
				SetLifeSpan(2.f);
			}
		}		
	}
}
