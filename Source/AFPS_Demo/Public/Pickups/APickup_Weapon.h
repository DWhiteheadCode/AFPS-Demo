#pragma once

#include "CoreMinimal.h"
#include "Pickups/APickupBase.h"

#include "GameplayTagContainer.h"

#include "APickup_Weapon.generated.h"

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAPickup_Weapon : public AAPickupBase
{
	GENERATED_BODY()
	
public:
	AAPickup_Weapon();

protected:
	// PICKUP LOGIC -------------------------------------------------------
	bool CanPickup(AActor* OtherActor) override;

	void Pickup(AActor* OtherActor) override;

	// WEAPON IDENTIFIER
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FGameplayTag WeaponIdentifier;
};
