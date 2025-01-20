#include "Dev/AI/AShootingDummyCharacter.h"

#include "Weapons/AWeaponContainerComponent.h"

AAShootingDummyCharacter::AAShootingDummyCharacter()
{
	WeaponComp = CreateDefaultSubobject<UAWeaponContainerComponent>("WeaponComp");
}

FVector AAShootingDummyCharacter::GetPawnViewLocation() const
{
	return GetActorLocation() + ViewpointOffset;
}