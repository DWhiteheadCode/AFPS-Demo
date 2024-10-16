#pragma once

#include "CoreMinimal.h"
#include "Weapons/AWeaponBase.h"
#include "AWeapon_RocketLauncher.generated.h"

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAWeapon_RocketLauncher : public AAWeaponBase
{
	GENERATED_BODY()
	
public:
	AAWeapon_RocketLauncher();

	void Fire_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category="Projectile")
	TSubclassOf<AActor> ProjectileClass;

};
