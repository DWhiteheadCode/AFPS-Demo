#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AShootingDummyCharacter.generated.h"

class UAWeaponContainerComponent;

UCLASS()
class AFPS_DEMO_API AAShootingDummyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAShootingDummyCharacter();

	virtual FVector GetPawnViewLocation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAWeaponContainerComponent> WeaponComp;
};
