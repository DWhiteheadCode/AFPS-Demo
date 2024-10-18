#include "Weapons/AWeaponContainerComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "Weapons/AWeaponBase.h"
#include "APlayerCharacter.h"

UAWeaponContainerComponent::UAWeaponContainerComponent()
{
	
}

void UAWeaponContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AAPlayerCharacter>(GetOwner());

	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Initialised WeaponContainerComponent [%s], but OwningCharacter was null"), *GetNameSafe(this));
		return;
	}

	SetupWeaponBindings();

	for (TSubclassOf<AAWeaponBase> WeaponClass : DefaultWeapons)
	{
		InstantiateWeapon(WeaponClass);
	}	

	EquipDefaultWeapon();
}

void UAWeaponContainerComponent::SetupWeaponBindings()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to set weapon bindings for [%s], but OwningCharacter was null"), *GetNameSafe(this));
		return;
	}

	if (APlayerController* const PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem
			= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(WeaponInputMappingContext, 0);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &UAWeaponContainerComponent::OnFireStart);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &UAWeaponContainerComponent::OnFireStop);

			// Weapon Switching
			EnhancedInputComponent->BindAction(EquipRocketAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipRocketInput);
			EnhancedInputComponent->BindAction(EquipLGAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipLGInput);
			EnhancedInputComponent->BindAction(EquipRailAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipRailInput);
		}
	}
}

bool UAWeaponContainerComponent::InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass)
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to instantiate weapon in WeaponContainerComponent [%s], but OwningCharacter was null"), *GetNameSafe(this));
		return false;
	}

	const FTransform SpawnTransform = FTransform(OwningCharacter->GetControlRotation(), OwningCharacter->GetActorLocation());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAWeaponBase* NewWeapon = GetWorld()->SpawnActor<AAWeaponBase>(WeaponClass, SpawnTransform, SpawnParams);

	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to instantiate weapon, but it was null"));
		return false;
	}

	// Check that NewWeapon's identifier is unique for this component
	if (GetWeapon(NewWeapon->GetIdentifier()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Not adding weapon [%s] as this WeaponContainer already has a weapon with that identifier."), *(NewWeapon->GetIdentifier().GetTagName().ToString()));
		//TODO Destroy NewWeapon?
		return false;
	}

	NewWeapon->SetOwningPlayer(OwningCharacter);
	Weapons.Add(NewWeapon);

	OnWeaponAdded.Broadcast(this, NewWeapon);

	return true;
}

void UAWeaponContainerComponent::OnFireStart()
{
	if (State == WeaponContainerState::NOT_EQUIPPED)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponContainer can't start fire while State is NOT_EQUIPPED"));
		return;
	}

	if (State == WeaponContainerState::READY && EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
	else
	{
		bShouldFireOnSwapEnd = true;
	}
}

void UAWeaponContainerComponent::OnFireStop()
{
	if (State == WeaponContainerState::NOT_EQUIPPED)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponContainer can't stop fire while State is NOT_EQUIPPED"));
		return;
	}

	if (State == WeaponContainerState::READY && EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
	else
	{
		bShouldFireOnSwapEnd = false;
	}
}

void UAWeaponContainerComponent::ProcessSwapInput(FGameplayTag WeaponIdentifier)
{
	// Must wait for the equip to complete before starting a swap to another weapon
	if (State == WeaponContainerState::EQUIPPING)
	{
		//UE_LOG(LogTemp, Log, TEXT("Ignoring weapon swap input as a weapon is already being equipped"));
		return;
	}

	// Ignore a swap if that weapon is already being swapped to
	if (WeaponToSwapTo && WeaponToSwapTo->GetIdentifier() == WeaponIdentifier)
	{
		//UE_LOG(LogTemp, Log, TEXT("Ignoring weapon swap input - already swapping to that weapon [%s]"), *(WeaponIdentifier.GetTagName().ToString()));
		return;
	}


	AAWeaponBase* Weapon = GetWeapon(WeaponIdentifier);
	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Receivied input to swap to [%s], but found no weapon matching that tag"), *(WeaponIdentifier.GetTagName().ToString()) )
		return;
	}

	WeaponToSwapTo = Weapon;

	// Don't start a new swap as one is already in progress - simply update target for the current swap
	if (State == WeaponContainerState::UNEQUIPPING)
	{
		UE_LOG(LogTemp, Log, TEXT("A weapon is already being unequipped. WeaponToSwapTo has been updated, but not starting new swap."));
		return;
	}	

	if (EquippedWeapon)
	{
		bShouldFireOnSwapEnd = EquippedWeapon->IsFiring();

		if (!EquippedWeapon->IsFiring())
		{
			UE_LOG(LogTemp, Log, TEXT("Starting swap immediately"));
			StartWeaponSwap();
		}
		else // Wait until EquippedWeapon finishes reloading to swap
		{
			const float PreSwapDelaySeconds = EquippedWeapon->GetRemainingFireDelay();
			EquippedWeapon->StopFire();

			UE_LOG(LogTemp, Log, TEXT("Starting swap in [%f] seconds"), PreSwapDelaySeconds);

			State = WeaponContainerState::WAITING_TO_UNEQUIP;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::StartWeaponSwap, PreSwapDelaySeconds);
		}
	}
	else // Equipping first weapon, ignore unequip delay
	{
		State = WeaponContainerState::EQUIPPING;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelaySeconds);
	}		
}

void UAWeaponContainerComponent::StartWeaponSwap()
{
	if (State == WeaponContainerState::UNEQUIPPING || State == WeaponContainerState::EQUIPPING)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to start weapon swap while already swapping weapon."));
		return;
	}

	if (!WeaponToSwapTo)
	{
		UE_LOG(LogTemp, Error, TEXT("Attempted to StartWeaponSwap() but WeaponToSwapTo was null"));
		return;
	}

	State = WeaponContainerState::UNEQUIPPING;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponUnequipDelayEnd, WeaponUnequipDelaySeconds);
}

void UAWeaponContainerComponent::OnWeaponUnequipDelayEnd()
{	
	if (State != WeaponContainerState::UNEQUIPPING)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponUnequipDelayEnd() called, but State wasn't UNEQUIPPING"));
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->UnequipWeapon();
	}

	State = WeaponContainerState::EQUIPPING;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelaySeconds);
}

void UAWeaponContainerComponent::OnWeaponEquipDelayEnd()
{
	if (State != WeaponContainerState::EQUIPPING)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponEquipDelayEnd() called, but State wasn't EQUIPPING"));
		return;
	}

	if (!WeaponToSwapTo)
	{
		UE_LOG(LogTemp, Error, TEXT("OnWeaponEquipDelayEnd() callled, but WeaponToSwapTo was null"));
		return;
	}	

	WeaponToSwapTo->EquipWeapon();

	EquippedWeapon = WeaponToSwapTo;
	WeaponToSwapTo = nullptr;

	if (bShouldFireOnSwapEnd)
	{
		EquippedWeapon->StartFire();
	}

	State = WeaponContainerState::READY;
}

void UAWeaponContainerComponent::EquipDefaultWeapon()
{
	if (Weapons.IsEmpty())
	{
		return;
	}

	AAWeaponBase* DefaultWeapon = Weapons[0];
	if (DefaultWeapon)
	{
		WeaponToSwapTo = DefaultWeapon;

		if (State == WeaponContainerState::NOT_EQUIPPED) // Equipping first weapon. Skip unequip time.
		{
			State = WeaponContainerState::EQUIPPING;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelaySeconds);
		}
		else
		{
			StartWeaponSwap();
		}		
	}	
}

void UAWeaponContainerComponent::OnEquipRocketInput()
{
	ProcessSwapInput(RocketGameplayTag);
}

void UAWeaponContainerComponent::OnEquipLGInput()
{
	ProcessSwapInput(LGGameplayTag);
}

void UAWeaponContainerComponent::OnEquipRailInput()
{
	ProcessSwapInput(RailGameplayTag);
}

AAWeaponBase* UAWeaponContainerComponent::GetWeapon(const FGameplayTag WeaponIdentifier) const
{
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == WeaponIdentifier)
		{
			return Weapon;
		}
	}

	return nullptr;
}