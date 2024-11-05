#pragma once

#include "CoreMinimal.h"
#include "Weapons/AWeaponBase.h"
#include "AWeapon_Rail.generated.h"

class UAStackComponent;

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAWeapon_Rail : public AAWeaponBase
{
	GENERATED_BODY()
	
public:
	AAWeapon_Rail();

	void Fire() override;

protected:
	UPROPERTY(EditAnywhere, Category="Damage")
	int Damage = 90;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float Range = 10000.f;

	UFUNCTION()
	TArray<FHitResult> PerformTrace();

	UFUNCTION()
	TArray<UAStackComponent*> GetStackComponentsFromHitResults(const TArray<FHitResult> HitResults) const;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawTrail(const FVector Start, const FVector End);

};
