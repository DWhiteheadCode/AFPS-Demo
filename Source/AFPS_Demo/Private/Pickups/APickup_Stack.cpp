#include "Pickups/APickup_Stack.h"

#include "AStackComponent.h"

bool AAPickup_Stack::CanPickup(AActor* OtherActor)
{
	if (!HasAuthority())
	{
		return false;
	}

	UAStackComponent* StackComp = Cast<UAStackComponent>(OtherActor->GetComponentByClass(UAStackComponent::StaticClass()));
	if (StackComp)
	{
		if (StackComp->CanAddHealth(HealthAmount, bCanOverhealHealth) || StackComp->CanAddArmour(ArmourAmount, bCanOverhealArmour))
		{
			return true;
		}
	}

	return false;
}

void AAPickup_Stack::Pickup(AActor* OtherActor)
{
	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Null OtherActor tried to Pickup() Stack-Pickup [%s]"), *GetNameSafe(this));
		return;
	}

	if (!CanPickup(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor [%s] tried to pickup StackPickup [%s], but CanPickup() was false."), *GetNameSafe(OtherActor), *GetNameSafe(this));
		return;
	}

	Super::Pickup(OtherActor);

	UAStackComponent* StackComp = Cast<UAStackComponent>(OtherActor->GetComponentByClass(UAStackComponent::StaticClass()));
	if (StackComp)
	{
		const bool bHealedHealth = StackComp->AddHealth(HealthAmount, bCanOverhealHealth, this);
		const bool bHealedArmour = StackComp->AddArmour(ArmourAmount, bCanOverhealArmour, this);

		if (!bHealedHealth && !bHealedArmour)
		{
			UE_LOG(LogTemp, Warning, TEXT("Stack pickup [%s] was picked up by [%s], but it didn't heal health or armour."), *GetNameSafe(this), *GetNameSafe(OtherActor));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stack pickup [%s] was picked up by [%s], but they didn't have a StackComponent."), *GetNameSafe(this), *GetNameSafe(OtherActor));
	}
}