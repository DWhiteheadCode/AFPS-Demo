#include "AWeaponBase.h"

#include "Components/StaticMeshComponent.h"

#include "APlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

AAWeaponBase::AAWeaponBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = MeshComp;
}

void AAWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (StartingAmmo > MaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon [%s] has more StartingAmmo [%d] than MaxAmmo [%d]"), *GetNameSafe(this), StartingAmmo, MaxAmmo);
	}

	Ammo = StartingAmmo;
}

bool AAWeaponBase::SetOwningPlayer(AAPlayerCharacter* InOwner)
{
	if (!InOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't set weapon owner to null"));
		return false;
	}

	OwningPlayer = InOwner;
	AttachToComponent(OwningPlayer->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_r"));

	return true;
}

void AAWeaponBase::EquipWeapon()
{
	MeshComp->SetVisibility(true, true);
}

void AAWeaponBase::UnequipWeapon()
{
	MeshComp->SetVisibility(false, true);
}

void AAWeaponBase::StartFire_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Firing base weapon"));
	Fire();
}

void AAWeaponBase::StopFire_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Stopping firing base weapon"));
}

bool AAWeaponBase::Fire_Implementation()
{
	if (Ammo <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Can't fire - out of ammo"));
		return false;
	}

	Ammo--;
	UE_LOG(LogTemp, Log, TEXT("Fired [%s] - Remaining Ammo: [%d]"), *GetNameSafe(this), Ammo);
	
	return true;
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}
