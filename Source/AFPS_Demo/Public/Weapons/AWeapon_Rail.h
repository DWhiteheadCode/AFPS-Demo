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

	void Fire_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category="Damage")
	int Damage = 90;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float Range = 10000.f;

	UFUNCTION()
	TArray<UAStackComponent*> GetStackComponentsInLine(const FVector StartPos, const FVector EndPos) const;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

};
