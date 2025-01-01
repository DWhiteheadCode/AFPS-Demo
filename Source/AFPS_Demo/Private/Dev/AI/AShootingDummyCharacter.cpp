#include "Dev/AI/AShootingDummyCharacter.h"

AAShootingDummyCharacter::AAShootingDummyCharacter()
{
	WeaponComp = CreateDefaultSubobject<UAWeaponContainerComponent>("WeaponComp");
}
