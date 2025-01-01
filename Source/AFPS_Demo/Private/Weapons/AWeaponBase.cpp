#include "Weapons/AWeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

#include "../AFPS_Demo.h"

AAWeaponBase::AAWeaponBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetVisibility(false, true); // Invisible while not equipped
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = MeshComp;

	AmbientAudioComp = CreateDefaultSubobject<UAudioComponent>("AmbientAudioComp");
	AmbientAudioComp->bAutoActivate = false;
	AmbientAudioComp->SetupAttachment(RootComponent);

	FiringAudioComp = CreateDefaultSubobject<UAudioComponent>("FiringAudioComp");
	FiringAudioComp->bAutoActivate = false;
	FiringAudioComp->SetupAttachment(RootComponent);

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

bool AAWeaponBase::SetOwningCharacter(ACharacter* InOwner)
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
		OnRep_OwningPlayer();
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to attach weapon [%s] to owner [%s]"), *GetNameSafe(this), *GetNameSafe(OwningPlayer));
	return false;
}

void AAWeaponBase::EquipWeapon()
{
	if (!bIsEquippable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to equip [%s], but bIsEquippable was false"), *GetNameSafe(this));
		return;
	}

	if (bIsEquipped)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon [%s] tried to equip while already equipped"), *GetNameSafe(this));
		return;
	}

	bIsEquipped = true;
	OnRep_IsEquipped();
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
		SetIsTriggerHeld(false);
	}

	bIsEquipped = false;
	OnRep_IsEquipped();
}

bool AAWeaponBase::IsEquippable() const
{
	return bIsEquippable;
}

void AAWeaponBase::SetIsEquippable(bool bInEquippable)
{
	bIsEquippable = bInEquippable;
	OnRep_IsEquippable();
}

void AAWeaponBase::OnRep_IsEquippable()
{
	OnIsEquippableChanged.Broadcast(this, bIsEquippable);
}

void AAWeaponBase::OnRep_IsEquipped()
{
	MeshComp->SetVisibility(bIsEquipped, true);
	OnIsEquippedChanged.Broadcast(this, bIsEquipped);

	if (AmbientAudioComp && AmbientAudioComp->Sound)
	{
		if (bIsEquipped)
		{
			AmbientAudioComp->Activate();
		}
		else
		{
			AmbientAudioComp->Deactivate();
		}
	}
}

// Note: It is the responsibility of the WeaponContainerComponent to call this from both the
// owning-client and the server
void AAWeaponBase::SetIsTriggerHeld(const bool bInTriggerHeld)
{
	bIsTriggerHeld = bInTriggerHeld;

	// Stop firing
	if (!bIsTriggerHeld)
	{
		bIsFiring = false;
		OnRep_IsFiring();

		return;
	}

	if (!CanFire())
	{
		return;
	}

	StartInitialFireDelay();
}

void AAWeaponBase::StartInitialFireDelay()
{
	// Get the InitialDelay before the first shot can be fired
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

	if (InitialDelay == 0.f)
	{
		OnInitialFireDelayEnd();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerHandle_FireDelay, this, &AAWeaponBase::OnInitialFireDelayEnd, InitialDelay);
	}
}

void AAWeaponBase::OnInitialFireDelayEnd()
{
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnInitialFireDelayEnd() called, but CanFire() was false"));
		return;
	}
	
	bIsFiring = true;
	OnRep_IsFiring();
	
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "Fire");
	GetWorldTimerManager().SetTimer(TimerHandle_FireDelay, Delegate, FireDelay, true, 0.f);
}

void AAWeaponBase::OnRep_IsFiring()
{
	if (!bIsFiring)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_FireDelay);
	}

	if (FiringAudioIsLoop())
	{
		if (bIsFiring)
		{
			StartFiringAudioLoop();
		}
		else
		{
			StopFiringAudioLoop();
		}
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

		if (!bInfiniteAmmo)
		{
			Ammo--;
			OnRep_Ammo(Ammo + 1);
		}		

		if (Ammo == 0)
		{
			bIsFiring = false;
			OnRep_IsFiring();
		}

		// Play the firing sound if firing audio is not a loop
		if (!FiringAudioIsLoop())
		{
			MulticastPlayNonLoopFireSound();
		}
	}
}

void AAWeaponBase::OnRep_Ammo(int OldAmmo)
{ 
	// Trigger was held while no ammo, but ammo was picked up so start firing.
	if (OldAmmo <= 0 && Ammo > 0 && bIsTriggerHeld)
	{
		if (CanFire())
		{
			StartInitialFireDelay();
		}		
	}

	OnAmmoChanged.Broadcast(this, Ammo, OldAmmo);
}

void AAWeaponBase::MulticastPlayNonLoopFireSound_Implementation()
{
	if (FiringAudioIsLoop())
	{
		UE_LOG(LogTemp, Warning, TEXT("MulticastPlayNonLoopFireSound() called, but firing audio is a loop"));
		return;
	}

	if (FiringAudioComp && FiringAudioComp->Sound)
	{
		FiringAudioComp->Play();
	}
}

void AAWeaponBase::StartFiringAudioLoop()
{
	if (FiringAudioIsLoop())
	{
		if (FiringAudioComp && FiringAudioComp->Sound)
		{
			FiringAudioComp->Play();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StartFiringAudioLoop() called on [%s], but FiringAudioIsLoop() returned false."), *GetNameSafe(this));
	}
}

void AAWeaponBase::StopFiringAudioLoop()
{
	if (FiringAudioIsLoop())
	{
		if (FiringAudioComp && FiringAudioComp->Sound)
		{
			FiringAudioComp->Stop();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StopFiringAudioLoop() called on [%s], but FiringAudioIsLoop() returned false."), *GetNameSafe(this));
	}
}

bool AAWeaponBase::FiringAudioIsLoop()
{
	if (FiringAudioComp && FiringAudioComp->Sound)
	{
		return FiringAudioComp->Sound->IsLooping();
	}

	return false;
}

bool AAWeaponBase::CanFire() const
{
	return Ammo > 0 || bInfiniteAmmo;
}

bool AAWeaponBase::IsFiring() const
{
	return bIsFiring;
}

bool AAWeaponBase::IsTriggerHeld() const
{
	return bIsTriggerHeld;
}

bool AAWeaponBase::IsEquipped() const
{
	return bIsEquipped;
}

bool AAWeaponBase::IsLocallyOwned() const
{
	if (OwningPlayer)
	{
		AController* Controller = OwningPlayer->GetController();

		if (Controller && Controller->IsLocalPlayerController())
		{
			return true;
		}
	}

	return false;
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}

int AAWeaponBase::GetAmmo() const
{
	return Ammo;
}

int AAWeaponBase::GetStartingAmmo() const
{
	return StartingAmmo;
}

int AAWeaponBase::GetMaxAmmo() const
{
	return MaxAmmo;
}

void AAWeaponBase::AddAmmo(const int InAmount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to add ammo to weapon [%s]"), *GetNameSafe(this));
		return;
	}

	if (InAmount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add negative [%d] ammo to weapon [%s]"), InAmount, *GetNameSafe(this));
		return;
	}	

	const int OldAmmo = Ammo;
	Ammo = FMath::Min(MaxAmmo, Ammo + InAmount);

	OnRep_Ammo(OldAmmo);
}

void AAWeaponBase::SetAmmo(const int InAmount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client attempted to set ammo for [%s]"), *GetNameSafe(this));
		return;
	}

	if (Ammo < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to set ammo for [%s] to a negative value [%d]."), *GetNameSafe(this), InAmount);
		return;
	}

	if (InAmount > MaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to set ammo for [%s] to [%d], which is greater than MaxAmmo [%d]."), *GetNameSafe(this), InAmount, MaxAmmo);
		return;
	}

	const int OldAmmo = Ammo;
	Ammo = InAmount;

	OnRep_Ammo(OldAmmo);
}

void AAWeaponBase::OnRep_OwningPlayer()
{
	// Weapon sounds should be played in 2D for the player holding the weapon
	if (OwningPlayer && OwningPlayer->IsLocallyControlled())
	{
		if (AmbientAudioComp) 
		{
			AmbientAudioComp->SetUISound(true);
		}

		if (FiringAudioComp)
		{
			FiringAudioComp->SetUISound(true);
		}
	}
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
	DOREPLIFETIME(AAWeaponBase, bIsEquippable);
	DOREPLIFETIME(AAWeaponBase, bIsEquipped);
	DOREPLIFETIME(AAWeaponBase, OwningPlayer);
}