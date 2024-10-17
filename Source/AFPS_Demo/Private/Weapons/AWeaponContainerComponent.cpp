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
	if (bIsUnequippingWeapon || bIsEquippingWeapon)
	{
		bShouldFireOnSwapEnd = true;
	}
	else if (EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
}

void UAWeaponContainerComponent::OnFireStop()
{
	if (bIsUnequippingWeapon || bIsEquippingWeapon)
	{
		bShouldFireOnSwapEnd = false;
	}
	else if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void UAWeaponContainerComponent::ProcessSwapInput(FGameplayTag WeaponIdentifier)
{
	// Must wait for the equip to complete before starting a swap to another weapon
	if (bIsEquippingWeapon)
	{
		UE_LOG(LogTemp, Log, TEXT("Ignoring weapon swap input as a weapon is already being equipped"));
		return;
	}

	// Ignore a swap to EquippedWeapon, unless a swap to another weapon had already started
	// I.e., if a weapon is being unequipped, it is still possible to overwrite the WeaponToEquip
	if (!bIsUnequippingWeapon)
	{
		if (EquippedWeapon && EquippedWeapon->GetIdentifier() == WeaponIdentifier)
		{
			UE_LOG(LogTemp, Log, TEXT("Ignoring weapon swap input as that weapon [%s] is already equipped"), *(EquippedWeapon->GetIdentifier().ToString()));
			return;
		}
	}

	AAWeaponBase* Weapon = GetWeapon(WeaponIdentifier);

	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Receivied input to swap to [%s], but found no weapon matching that tag"), *(WeaponIdentifier.GetTagName().ToString()) )
		return;
	}

	WeaponToSwapTo = Weapon;

	// Don't start a swap as one is already in progress
	if (bIsUnequippingWeapon)
	{
		UE_LOG(LogTemp, Log, TEXT("A weapon is already being unequipped. WeaponToSwapTo has been updated, but not starting new swap."));
		return;
	}	
	
	if (EquippedWeapon)
	{
		// Used to preserve firing if trigger is held through weapon switch
		bShouldFireOnSwapEnd = EquippedWeapon->IsFiring();
		if (EquippedWeapon->IsFiring())
		{
			EquippedWeapon->StopFire();
		}
			
		// Don't start swapping until EquippedWeapon finishes reloading from its last fired shot
		const float PreSwapDelay = EquippedWeapon->GetRemainingFireDelay();

		if (PreSwapDelay == 0.f)
		{
			StartWeaponSwap();
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::StartWeaponSwap, PreSwapDelay);
		}		
	}
	else // Equipping first weapon, ignore unequip
	{
		bIsEquippingWeapon = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelay);
	}		
}

void UAWeaponContainerComponent::StartWeaponSwap()
{
	if (bIsUnequippingWeapon || bIsEquippingWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to start weapon swap while already swapping weapon."));
		return;
	}

	if (!WeaponToSwapTo)
	{
		UE_LOG(LogTemp, Error, TEXT("Attempted to StartWeaponSwap() but WeaponToSwapTo was null"));
		return;
	}

	bIsUnequippingWeapon = true;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponUnequipDelayEnd, WeaponUnequipDelay);
}

void UAWeaponContainerComponent::OnWeaponUnequipDelayEnd()
{	
	if (!bIsUnequippingWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponUnequipDelayEnd() called, but bIsUnequippingWeapon was false"));
		return;
	}

	if (bIsEquippingWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponUnequipDelayEnd() called, but bIsEquippingWeapon was true"));
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->UnequipWeapon();
	}

	bIsUnequippingWeapon = false;
	bIsEquippingWeapon = true;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelay);
}

void UAWeaponContainerComponent::OnWeaponEquipDelayEnd()
{
	if (!WeaponToSwapTo)
	{
		UE_LOG(LogTemp, Error, TEXT("OnWeaponEquipDelayEnd() callled, but WeaponToSwapTo was null"));
		return;
	}

	if (!bIsEquippingWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponEquipDelayEnd() called, but bIsEquippingWeapon was false"));
		return;
	}

	WeaponToSwapTo->EquipWeapon();

	EquippedWeapon = WeaponToSwapTo;
	WeaponToSwapTo = nullptr;

	if (bShouldFireOnSwapEnd)
	{
		EquippedWeapon->StartFire();
	}

	bIsEquippingWeapon = false;
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
		bIsEquippingWeapon = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelay);
	}	
}

TArray<AAWeaponBase*> UAWeaponContainerComponent::GetWeapons() const
{
	return Weapons;
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

AAWeaponBase* UAWeaponContainerComponent::GetWeapon(FGameplayTag WeaponIdentifier) const
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