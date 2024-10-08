#pragma once

#include "CoreMinimal.h"
#include "ABasePickup.h"
#include "AStackPickup.generated.h"

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAStackPickup : public AABasePickup
{
	GENERATED_BODY()
	
protected:
	void OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(EditAnywhere, Category="Health")
	int HealthAmount = 25;

	UPROPERTY(EditAnywhere, Category = "Health")
	bool bCanOverhealHealth = false;

	UPROPERTY(EditAnywhere, Category = "Health")
	int ArmourAmount = 0;

	UPROPERTY(EditAnywhere, Category = "Health")
	bool bCanOverhealArmour = false;
};
