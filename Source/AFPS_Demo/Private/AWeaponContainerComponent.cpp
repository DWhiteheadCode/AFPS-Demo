#include "AWeaponContainerComponent.h"

#include "AWeaponBase.h"
#include "APlayerCharacter.h"

UAWeaponContainerComponent::UAWeaponContainerComponent()
{
	OwningCharacter = Cast<AAPlayerCharacter>(GetOwner());
}

void UAWeaponContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TSubclassOf<AAWeaponBase> WeaponClass : DefaultWeapons)
	{
		InstantiateWeapon(WeaponClass);
	}	

	// Equip the first weapon by default
	if (Weapons.Num() >= 1)
	{
		Weapons[0]->EquipWeapon(OwningCharacter);
	}
}

bool UAWeaponContainerComponent::InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass)
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add Weapon to WeaponComponent, but OwningCharacter was null"));
		return false;
	}

	FTransform SpawnTransform = FTransform(OwningCharacter->GetControlRotation(), OwningCharacter->GetActorLocation());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAWeaponBase* NewWeapon = GetWorld()->SpawnActor<AAWeaponBase>(WeaponClass, SpawnTransform, SpawnParams);

	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to spawn NewWeapon, but it was null"));
		return false;
	}

	// Check that NewWeapon's identifier is unique for this component
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == NewWeapon->GetIdentifier())
		{
			UE_LOG(LogTemp, Log, TEXT("Not adding new weapon as this component already has a weapon with matching identifier"));
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Instantiated Weapon"));
	Weapons.Add(NewWeapon);
	return true;
}



