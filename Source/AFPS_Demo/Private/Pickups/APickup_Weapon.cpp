#include "Pickups/APickup_Weapon.h"

#include "Weapons/AWeaponContainerComponent.h"
#include "Weapons/AWeaponBase.h"

AAPickup_Weapon::AAPickup_Weapon()
{
	CooldownDuration = 5.f;
	bRespawns = true;
}

bool AAPickup_Weapon::CanPickup(AActor* OtherActor)
{
	if (!Super::CanPickup(OtherActor))
	{
		return false;
	}

	if (!OtherActor) // Redundant check due to Super::CanPickup(), but leaving here as sanity check.
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

	AAWeaponBase* Weapon = WeaponComp->GetWeapon(WeaponIdentifier);
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] picked up [%s], but their WeaponComp didn't have a Weapon matching the ID [%s]"), *GetNameSafe(OtherActor), *GetNameSafe(this), *(WeaponIdentifier.ToString()));
		return;
	}

	if (Weapon->IsEquippable()) // OtherActor already has this weapon, so add ammo
	{
		if (Weapon->GetAmmo() < Weapon->GetStartingAmmo())
		{
			Weapon->SetAmmo( Weapon->GetStartingAmmo() );
		}
		else
		{
			Weapon->AddAmmo(AmmoIncrement);
		}
	}
	else
	{
		Weapon->SetIsEquippable(true);
	}
}
