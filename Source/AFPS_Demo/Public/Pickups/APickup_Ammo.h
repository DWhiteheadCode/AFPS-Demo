#pragma once

#include "CoreMinimal.h"
#include "Pickups/APickupBase.h"

#include "GameplayTagContainer.h"

#include "APickup_Ammo.generated.h"

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAPickup_Ammo : public AAPickupBase
{
	GENERATED_BODY()
	
public:
	AAPickup_Ammo();

	bool CanPickup(AActor* OtherActor) override;

	void Pickup(AActor* OtherActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FGameplayTag WeaponIdentifier;

	UPROPERTY(EditDefaultsOnly, Category="Ammo")
	int AmmoIncrement = 1;
};
