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
	// COLLISION ----------------------------------------------------------
	void OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	// HEALTH -------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category="Health")
	int HealthAmount = 25;

	UPROPERTY(EditAnywhere, Category = "Health")
	bool bCanOverhealHealth = false;

	// ARMOUR --------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Health")
	int ArmourAmount = 0;

	UPROPERTY(EditAnywhere, Category = "Health")
	bool bCanOverhealArmour = false;
};
