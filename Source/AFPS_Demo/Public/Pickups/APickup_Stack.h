#pragma once

#include "CoreMinimal.h"
#include "Pickups/APickupBase.h"
#include "APickup_Stack.generated.h"

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAPickup_Stack : public AAPickupBase
{
	GENERATED_BODY()
	
protected:
	// PICKUP LOGIC -------------------------------------------------------
	bool CanPickup(AActor* OtherActor) override;

	void Pickup(AActor* OtherActor) override;

	// HEALTH -------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category="Stack")
	int HealthAmount = 25;

	UPROPERTY(EditAnywhere, Category = "Stack")
	bool bCanOverhealHealth = false;

	// ARMOUR --------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Stack")
	int ArmourAmount = 0;

	UPROPERTY(EditAnywhere, Category = "Stack")
	bool bCanOverhealArmour = false;
};
