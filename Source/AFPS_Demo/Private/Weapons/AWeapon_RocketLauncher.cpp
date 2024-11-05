#include "Weapons/AWeapon_RocketLauncher.h"

#include "APlayerCharacter.h"

AAWeapon_RocketLauncher::AAWeapon_RocketLauncher()
{
	StartingAmmo = 15;
	MaxAmmo = 25;

	FireDelay = 0.8f;
}

void AAWeapon_RocketLauncher::Fire_Implementation()
{
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}

	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to fire [%s], but OwningPlayer was null"), *GetNameSafe(this));
		return;
	}

	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile Class not set for [%s]"), *GetNameSafe(this));
		return;
	}	

	Super::Fire_Implementation();

	if (HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = OwningPlayer;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FRotator EyeRotation = OwningPlayer->GetControlRotation();
		const FVector EyeLocation = OwningPlayer->GetPawnViewLocation();

		const FTransform SpawnTM = FTransform(EyeRotation, EyeLocation);

		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
	}	
}
