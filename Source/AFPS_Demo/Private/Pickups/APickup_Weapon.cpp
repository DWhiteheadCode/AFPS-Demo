#include "Pickups/APickup_Weapon.h"

#include "Weapons/AWeaponContainerComponent.h"

bool AAPickup_Weapon::CanPickup(AActor* OtherActor)
{
	if (!Super::CanPickup(OtherActor))
	{
		return false;
	}

	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanPickup() called on [%s] with null OtherActor"), *GetNameSafe(this));
		return false;
	}

	UAWeaponContainerComponent* WeaponComp = Cast<UAWeaponContainerComponent>(OtherActor->GetComponentByClass(UAWeaponContainerComponent::StaticClass()));
	if (!WeaponComp)
	{
		return false;
	}

	return WeaponComp->HasWeapon(WeaponIdentifier);
}

void AAPickup_Weapon::Pickup(AActor* OtherActor)
{
	if (!CanPickup(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup() called on [%s], but CanPickup() returned false"), *GetNameSafe(this));
		return;
	}

	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup() called on [%s], but OtherActor was null"), *GetNameSafe(this));
		return;
	}

	Super::Pickup(OtherActor);

	UAWeaponContainerComponent* WeaponComp = Cast<UAWeaponContainerComponent>(OtherActor->GetComponentByClass(UAWeaponContainerComponent::StaticClass()));
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] picked up [%s], but they didn't have a WeaponContainerComponent"), *GetNameSafe(OtherActor), *GetNameSafe(this));
		return;
	}

	if (!WeaponComp->MakeEquippable(WeaponIdentifier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup [%s] failed to make weapon [%s] equippable"), *GetNameSafe(this), *(WeaponIdentifier.ToString()));
	}
}
