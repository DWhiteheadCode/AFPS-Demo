#include "AWeaponContainerComponent.h"

#include "AWeaponBase.h"
#include "APlayerCharacter.h"

UAWeaponContainerComponent::UAWeaponContainerComponent()
{
	
}

void UAWeaponContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AAPlayerCharacter>(GetOwner());

	for (TSubclassOf<AAWeaponBase> WeaponClass : DefaultWeapons)
	{
		InstantiateWeapon(WeaponClass);
	}	

	EquipDefaultWeapon();
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
	SpawnParams.Owner = OwningCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAWeaponBase* NewWeapon = GetWorld()->SpawnActor<AAWeaponBase>(WeaponClass, SpawnTransform, SpawnParams);

	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to spawn NewWeapon, but it was null"));
		return false;
	}

	// Check that NewWeapon's identifier is unique for this component
	// TODO Lok for a better way to do this (currently O(n), same as other operations on Weapons array)
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == NewWeapon->GetIdentifier())
		{
			UE_LOG(LogTemp, Log, TEXT("Not adding new weapon as this component already has a weapon with matching identifier"));
			return false;
		}
	}

	NewWeapon->SetOwningPlayer(OwningCharacter);
	Weapons.Add(NewWeapon);
	return true;
}

// TODO This is O(n). Could possibly use a Map, though they don't replicate
// Given n is 9 at most, might be ok to leave as is- need to test performance.
void UAWeaponContainerComponent::EquipWeapon(FGameplayTag InIdentifier)
{
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == InIdentifier)
		{
			Weapon->EquipWeapon();
		}
	}
}

void UAWeaponContainerComponent::EquipDefaultWeapon()
{
	if (Weapons.IsEmpty())
	{
		return;
	}

	AAWeaponBase* DefaultWeapon = Weapons[0];
	EquipWeapon(DefaultWeapon->GetIdentifier());
}



