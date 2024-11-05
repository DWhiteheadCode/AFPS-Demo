#pragma once

#include "CoreMinimal.h"
#include "Weapons/AWeaponBase.h"
#include "AWeapon_LG.generated.h"

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAWeapon_LG : public AAWeaponBase
{
	GENERATED_BODY()
	
public:
	AAWeapon_LG();

	void Fire_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Damage")
	int Damage = 7;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float Range = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UFUNCTION()
	FHitResult PerformTrace();

};
