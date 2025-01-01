#include "Dev/AI/AShootingDummyCharacter.h"

AAShootingDummyCharacter::AAShootingDummyCharacter()
{
	WeaponComp = CreateDefaultSubobject<UAWeaponContainerComponent>("WeaponComp");
}

FVector AAShootingDummyCharacter::GetPawnViewLocation() const
{
	return GetActorLocation() + FVector(0, 0, 90);
}
