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

void AAWeaponBase::StartFire()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_FireDelay))
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start weapon firing while already firing"));
		return;
	}

	float InitialDelay = 0.f;

	if (LastFireTime >= 0) // Weapon has been fired before
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		const float TimeSinceLastShot = CurrentTime - LastFireTime;
		
		if (TimeSinceLastShot < FireDelay) // Weapon is still "reloading" (between bullets)
		{
			InitialDelay = FireDelay - TimeSinceLastShot;
		}
	}

	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "OnFireDelayEnd");
	GetWorldTimerManager().SetTimer(TimerHandle_FireDelay, Delegate, FireDelay, true, InitialDelay);
}

void AAWeaponBase::StopFire()
{
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle_FireDelay))
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to stop weapon firing while not already firing"));
		return;
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_FireDelay);
}

void AAWeaponBase::Fire_Implementation()
{
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire weapon [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}
	
	Ammo--;
	LastFireTime = GetWorld()->GetTimeSeconds();
	UE_LOG(LogTemp, Log, TEXT("Fired [%s] - Remaining Ammo: [%d]"), *GetNameSafe(this), Ammo);
}

bool AAWeaponBase::CanFire()
{
	return Ammo > 0;
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}

void AAWeaponBase::OnFireDelayEnd()
{
	// Setting this (even if the weapon can't' fire) means it can't attempt to fire again too soon
	LastFireTime = GetWorld()->GetTimeSeconds();

	if (CanFire())
	{
		Fire();
	}
}
