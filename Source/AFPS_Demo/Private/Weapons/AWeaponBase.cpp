#include "Weapons/AWeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

#include "APlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

#include "../AFPS_Demo.h"

AAWeaponBase::AAWeaponBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetVisibility(false, true); // Invisible while not equipped
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = MeshComp;

	bReplicates = true;
}

void AAWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (StartingAmmo > MaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon [%s] has more StartingAmmo [%d] than MaxAmmo [%d]"), *GetNameSafe(this), StartingAmmo, MaxAmmo);
	}

	if (HasAuthority())
	{
		Ammo = StartingAmmo;
	}	
}

bool AAWeaponBase::SetOwningPlayer(AAPlayerCharacter* InOwner)
{
	if (!InOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to set weapon [%s] owner to null"), *GetNameSafe(this));
		return false;
	}

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to change owner of weapon: [%s] to [%s]."), *GetNameSafe(this), *GetNameSafe(InOwner));
		return false;
	}

	USkeletalMeshComponent* InMesh = InOwner->GetMesh();

	if (!InMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to attach weapon [%s] to new owner ([%s]), but new owner had no mesh."), *GetNameSafe(this), *GetNameSafe(InOwner));
		return false;
	}

	if (AttachToComponent( InMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_r")) )
	{
		OwningPlayer = InOwner;
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to attach weapon [%s] to owner [%s]"), *GetNameSafe(this), *GetNameSafe(OwningPlayer));
	return false;
}

void AAWeaponBase::EquipWeapon()
{
	if (bIsEquipped)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon [%s] tried to equip while already equipped"), *GetNameSafe(this));
		return;
	}

	bIsEquipped = true;
	OnRep_IsEquippedChanged();
}

void AAWeaponBase::UnequipWeapon()
{
	if (!bIsEquipped)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnequipWeapon() called on Weapon [%s] while already unequipped"), *GetNameSafe(this));
		return;
	}
	
	if (bIsFiring)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon [%s] tried to unequip but it was still firing. Stopping fire."), *GetNameSafe(this));
		StopFire();
	}

	bIsEquipped = false;
	OnRep_IsEquippedChanged();
}

void AAWeaponBase::OnRep_IsEquippedChanged()
{
	MeshComp->SetVisibility(bIsEquipped, true);
	OnEquipStateChanged.Broadcast(this, bIsEquipped);
}

// Note: It is the responsibility of the WeaponContainerComponent to call StartFire() from both the
// client and the server
void AAWeaponBase::StartFire()
{
	bIsFiring = true;
	float InitialDelay = 0.f;

	if (LastFireTime >= 0) // Weapon has been fired before
	{
		AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();

		if (!GameState)
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempted to StartFire(), but GameState was null. Assuming CurrentTime == LastFireTime."));
		}

		// Try to get the CurrentTime (server). If this fails (as GameState hasn't replicated), assume the weapon was just fired
		const float CurrentTime = (GameState) ? GameState->GetServerWorldTimeSeconds() : LastFireTime;
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

// Note: It is the responsibility of the WeaponContainerComponent to call StopFire() from both the
// client and the server
void AAWeaponBase::StopFire()
{
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_FireDelay);
}

void AAWeaponBase::OnRep_IsFiring()
{
	if (bIsFiring)
	{
		StartFire();
	}
	else
	{
		StopFire();
	}
}

void AAWeaponBase::Fire()
{
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire weapon [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}
	
	if (HasAuthority())
	{
		AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();

		if (!GameState)
		{
			UE_LOG(LogTemp, Error, TEXT("Server failed to retrieve GameState in [%s]::Fire(). Weapon won't fire."), *GetNameSafe(this));
			return;
		}
		
		LastFireTime = GameState->GetServerWorldTimeSeconds();
		Ammo--;

		ClientAmmoChanged(Ammo, Ammo + 1);
	}
}

void AAWeaponBase::ClientAmmoChanged_Implementation(const int NewAmmo, const int OldAmmo)
{
	OnAmmoChanged.Broadcast(this, NewAmmo, OldAmmo);
}

void AAWeaponBase::OnFireDelayEnd()
{
	if (HasAuthority())
	{
		AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();

		if (!GameState)
		{
			UE_LOG(LogTemp, Error, TEXT("Server failed to retrieve GameState in [%s]::OnFireDelayEnd(). Weapon won't fire."), *GetNameSafe(this));
			return;
		}

		// Setting this (even if the weapon can't fire) means it can't attempt to fire again too soon
		LastFireTime = GameState->GetServerWorldTimeSeconds();
	}

	if (CanFire())
	{
		Fire();
	}
}

bool AAWeaponBase::CanFire() const
{
	return Ammo > 0;
}

bool AAWeaponBase::IsFiring() const
{
	return bIsFiring;
}

bool AAWeaponBase::IsEquipped() const
{
	return bIsEquipped;
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}

int AAWeaponBase::GetAmmo() const
{
	return Ammo;
}

int AAWeaponBase::GetMaxAmmo() const
{
	return MaxAmmo;
}

float AAWeaponBase::GetRemainingFireDelay() const
{
	const float RemainingDelay = GetWorldTimerManager().GetTimerRemaining(TimerHandle_FireDelay);

	return (RemainingDelay >= 0.f) ? RemainingDelay : 0.f;
}

void AAWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAWeaponBase, Ammo);
	DOREPLIFETIME(AAWeaponBase, LastFireTime);
	DOREPLIFETIME(AAWeaponBase, bIsFiring);
	DOREPLIFETIME(AAWeaponBase, bIsEquipped);
	DOREPLIFETIME(AAWeaponBase, OwningPlayer);
}