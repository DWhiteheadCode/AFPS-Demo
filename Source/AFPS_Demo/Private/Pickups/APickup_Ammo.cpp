#include "Pickups/APickup_Ammo.h"

#include "Weapons/AWeaponContainerComponent.h"
#include "Weapons/AWeaponBase.h"

AAPickup_Ammo::AAPickup_Ammo()
{
	bRespawns = true;
	CooldownDuration = 15.f;
}

bool AAPickup_Ammo::CanPickup(AActor* OtherActor)
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

	AAWeaponBase* Weapon = WeaponComp->GetWeapon(WeaponIdentifier);
	if (!Weapon)
	{
		return false;
	}
	
	return Weapon->GetAmmo() < Weapon->GetMaxAmmo();
}

void AAPickup_Ammo::Pickup(AActor* OtherActor)
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

	Weapon->AddAmmo(AmmoIncrement);
}
