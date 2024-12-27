#include "Weapons/AWeaponContainerComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapons/AWeaponBase.h"
#include "APlayerCharacter.h"

#include "../AFPS_Demo.h"

UAWeaponContainerComponent::UAWeaponContainerComponent()
{
	SetIsReplicatedByDefault(true);
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

	if (OwningCharacter->IsLocallyControlled())
	{
		SetupWeaponBindings();
	}	

	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<AAWeaponBase> WeaponClass : DefaultWeapons)
		{
			InstantiateWeapon(WeaponClass);
		}

		EquipDefaultWeapon();
	}
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
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &UAWeaponContainerComponent::OnTriggerHeldInput);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &UAWeaponContainerComponent::OnTriggerReleasedInput);

			// Weapon Switching
			EnhancedInputComponent->BindAction(EquipRocketAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipRocketInput);
			EnhancedInputComponent->BindAction(EquipLGAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipLGInput);
			EnhancedInputComponent->BindAction(EquipRailAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipRailInput);
		}
	}
}

void UAWeaponContainerComponent::OnRep_Weapons()
{
	OnWeaponsReplicated.Broadcast(this);
}

bool UAWeaponContainerComponent::InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass)
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to instantiate weapon in WeaponContainerComponent [%s], but OwningCharacter was null"), *GetNameSafe(this));
		return false;
	}

	if (!OwningCharacter->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to instantiate weapon in component [%s]."), *GetNameSafe(this));
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
		NewWeapon->SetLifeSpan(1.f);
		return false;
	}

	NewWeapon->SetOwningPlayer(OwningCharacter);
	Weapons.Add(NewWeapon);

	OnRep_Weapons(); // Server should notify its delegate too
	//ClientOnWeaponAdded(NewWeapon); // Only the owner of the weapon needs to update their UI

	return true;
}

void UAWeaponContainerComponent::OnTriggerHeldInput()
{
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::NOT_EQUIPPED)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponContainer can't start fire while WeaponEquipState is NOT_EQUIPPED"));
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerOnTriggerHeldInput();
	}

	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::READY && EquippedWeapon) // Start firing
	{
		EquippedWeapon->SetIsTriggerHeld(true);
	}
	else // Prepare to fire when the swap ends
	{
		RepData_WeaponSwap.bShouldFireOnSwapEnd = true;
	}
}

void UAWeaponContainerComponent::ServerOnTriggerHeldInput_Implementation()
{
	OnTriggerHeldInput();
}

void UAWeaponContainerComponent::OnTriggerReleasedInput()
{
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::NOT_EQUIPPED)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponContainer can't stop fire while WeaponEquipState is NOT_EQUIPPED"));
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerOnTriggerReleasedInput();
	}

	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::READY && EquippedWeapon)
	{
		EquippedWeapon->SetIsTriggerHeld(false);
	}
	else
	{
		RepData_WeaponSwap.bShouldFireOnSwapEnd = false;
	}
}

void UAWeaponContainerComponent::ServerOnTriggerReleasedInput_Implementation()
{
	OnTriggerReleasedInput();
}

void UAWeaponContainerComponent::ProcessSwapInput(FGameplayTag WeaponIdentifier)
{
	// Must wait for the equip to complete before starting a swap to another weapon
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::EQUIPPING)
	{
		//UE_LOG(LogTemp, Log, TEXT("Ignoring weapon swap input as a weapon is already being equipped"));
		return;
	}

	// If a swap isn't already occuring, ignore a swap to the EquippedWeapon
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::READY)
	{
		if (EquippedWeapon && EquippedWeapon->GetIdentifier() == WeaponIdentifier)
		{
			return;
		}
	}

	// Ignore a swap if that weapon is already being swapped to
	if (RepData_WeaponSwap.WeaponToSwapTo && RepData_WeaponSwap.WeaponToSwapTo->GetIdentifier() == WeaponIdentifier)
	{
		//UE_LOG(LogTemp, Log, TEXT("Ignoring weapon swap input - already swapping to that weapon [%s]"), *(WeaponIdentifier.GetTagName().ToString()));
		return;
	}

	AAWeaponBase* Weapon = GetWeapon(WeaponIdentifier);
	if (!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Receivied input to swap to [%s], but found no weapon matching that tag"), *(WeaponIdentifier.GetTagName().ToString()));
		return;
	}

	if (!Weapon->IsEquippable()) 
	{
		return;
	}

	if (!GetOwner()->HasAuthority()) // Let client visually start swapping weapon, but tell server to start swap as well
	{
		ServerProcessSwapInput(WeaponIdentifier);
	}
 
	RepData_WeaponSwap.WeaponToSwapTo = Weapon;

	// Don't start a new swap as one is already in progress - target for the current swap has simply been updated
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::UNEQUIPPING || RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::WAITING_TO_UNEQUIP)
	{
		UE_LOG(LogTemp, Log, TEXT("A weapon is already being unequipped. WeaponToSwapTo has been updated, but not starting new swap."));
		return;
	}

	if (EquippedWeapon)
	{
		const float PreSwapDelaySeconds = EquippedWeapon->GetRemainingFireDelay();
		RepData_WeaponSwap.bShouldFireOnSwapEnd = EquippedWeapon->IsTriggerHeld();

		if (EquippedWeapon->IsTriggerHeld())
		{
			EquippedWeapon->SetIsTriggerHeld(false);
		}
		
		if (PreSwapDelaySeconds > 0.f) // Wait until EquippedWeapon finishes reloading to start the swap
		{
			RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::WAITING_TO_UNEQUIP;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::StartWeaponSwap, PreSwapDelaySeconds);
		}
		else
		{
			StartWeaponSwap();
		}
	}
	else // Equipping first weapon, ignore unequip delay
	{
		RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::EQUIPPING;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelaySeconds);
	}
}

void UAWeaponContainerComponent::ServerProcessSwapInput_Implementation(FGameplayTag WeaponIdentifier)
{
	ProcessSwapInput(WeaponIdentifier);
}

void UAWeaponContainerComponent::StartWeaponSwap()
{
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::UNEQUIPPING || RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::EQUIPPING)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to start weapon swap while already swapping weapon."));
		return;
	}

	if (!RepData_WeaponSwap.WeaponToSwapTo)
	{
		LogOnScreen(this, FString("Attempted to StartWeaponSwap, but WeaponToSwapTo was null"));
		//UE_LOG(LogTemp, Error, TEXT("Attempted to StartWeaponSwap() but WeaponToSwapTo was null"));
		return;
	}

	// Player started a swap, but chose to stay with EquippedWeapon before the swap started (e.g. while it was reloading)
	if (EquippedWeapon == RepData_WeaponSwap.WeaponToSwapTo)
	{
		LogOnScreen(this, FString("Cancelling swap before it starts"));
		//UE_LOG(LogTemp, Log, TEXT("Cancelling swap before it starts as WeaponToSwapTo == EquippedWeapon"));
		RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::READY;
		RepData_WeaponSwap.WeaponToSwapTo = nullptr;
		return;
	}

	// First weapon, skip unequip delay
	if (RepData_WeaponSwap.WeaponEquipState == WeaponEquipState::NOT_EQUIPPED) 
	{
		RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::EQUIPPING;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelaySeconds);
	}
	else // Standard 
	{
		RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::UNEQUIPPING;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponUnequipDelayEnd, WeaponUnequipDelaySeconds);
	}	
}

void UAWeaponContainerComponent::OnWeaponUnequipDelayEnd()
{	
	if (RepData_WeaponSwap.WeaponEquipState != WeaponEquipState::UNEQUIPPING)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponUnequipDelayEnd() called, but WeaponEquipState wasn't UNEQUIPPING"));
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->UnequipWeapon();
		// Don't set EquippedWeapon to null here as this is the weapon that should still be dropped if the player dies before the equip finishes
	}

	RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::EQUIPPING;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WeaponSwap, this, &UAWeaponContainerComponent::OnWeaponEquipDelayEnd, WeaponEquipDelaySeconds);
}

void UAWeaponContainerComponent::OnWeaponEquipDelayEnd()
{
	if (RepData_WeaponSwap.WeaponEquipState != WeaponEquipState::EQUIPPING)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponEquipDelayEnd() called, but WeaponEquipState wasn't EQUIPPING"));
		return;
	}

	if (!RepData_WeaponSwap.WeaponToSwapTo)
	{
		UE_LOG(LogTemp, Error, TEXT("OnWeaponEquipDelayEnd() callled, but WeaponToSwapTo was null"));
		return;
	}	

	RepData_WeaponSwap.WeaponToSwapTo->EquipWeapon();

	EquippedWeapon = RepData_WeaponSwap.WeaponToSwapTo;
	RepData_WeaponSwap.WeaponToSwapTo = nullptr;

	if (RepData_WeaponSwap.bShouldFireOnSwapEnd)
	{
		EquippedWeapon->SetIsTriggerHeld(true);
	}

	RepData_WeaponSwap.WeaponEquipState = WeaponEquipState::READY;
}

void UAWeaponContainerComponent::EquipDefaultWeapon()
{
	if (Weapons.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to equip default weapon- Weapons was empty."));
		return;
	}

	AAWeaponBase* DefaultWeapon = Weapons[0];
	DefaultWeapon->SetIsEquippable(true);
	ProcessSwapInput(DefaultWeapon->GetIdentifier());

	UE_LOG(LogTemp, Log, TEXT("Making DefaultWeapon [%s] equippable."), *GetNameSafe(DefaultWeapon));
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

/*
 * TODO - This is obviously O(n), which is far from ideal. 
 * Need to test actual time of O(n) approach (given n is at most 8), versus setting
 * up a TMap for lookups, while also maintaining TArray for replication.
 */
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

bool UAWeaponContainerComponent::HasWeapon(FGameplayTag WeaponIdentifier) const
{
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == WeaponIdentifier)
		{
			return true;
		}
	}

	return false;
}

bool UAWeaponContainerComponent::MakeEquippable(FGameplayTag WeaponIdentifier)
{
	AAWeaponBase* Weapon = GetWeapon(WeaponIdentifier);

	if (Weapon)
	{
		Weapon->SetIsEquippable(true);
		return true;
	}

	return false;
}

/*
void UAWeaponContainerComponent::ClientOnWeaponAdded_Implementation(AAWeaponBase* NewWeapon)
{
	//LogOnScreen(this, FString("ClientOnWeaponAdded()"));
	OnWeaponAdded.Broadcast(this, NewWeapon);
}
*/

void UAWeaponContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAWeaponContainerComponent, Weapons);
	DOREPLIFETIME(UAWeaponContainerComponent, EquippedWeapon);
	DOREPLIFETIME(UAWeaponContainerComponent, RepData_WeaponSwap);
}