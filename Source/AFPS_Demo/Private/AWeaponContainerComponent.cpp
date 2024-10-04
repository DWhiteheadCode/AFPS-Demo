#include "AWeaponContainerComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "AWeaponBase.h"
#include "APlayerCharacter.h"

UAWeaponContainerComponent::UAWeaponContainerComponent()
{
	
}

void UAWeaponContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AAPlayerCharacter>(GetOwner());

	// Set up weapon switch bindings
	if (OwningCharacter)
	{
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
				EnhancedInputComponent->BindAction(EquipRocketAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipWeaponInput);
				EnhancedInputComponent->BindAction(EquipLGAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipWeaponInput);
				EnhancedInputComponent->BindAction(EquipRailAction, ETriggerEvent::Triggered, this, &UAWeaponContainerComponent::OnEquipWeaponInput);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Initialised WeaponContainerComponent, but OwningCharacter was null"));
	}

	for (TSubclassOf<AAWeaponBase> WeaponClass : DefaultWeapons)
	{
		InstantiateWeapon(WeaponClass);
	}	

	EquipDefaultWeapon();
}

bool UAWeaponContainerComponent::InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass)
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add Weapon to WeaponComponent, but OwningCharacter was null"));
		return false;
	}

	FTransform SpawnTransform = FTransform(OwningCharacter->GetControlRotation(), OwningCharacter->GetActorLocation());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAWeaponBase* NewWeapon = GetWorld()->SpawnActor<AAWeaponBase>(WeaponClass, SpawnTransform, SpawnParams);

	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to spawn NewWeapon, but it was null"));
		return false;
	}

	// Check that NewWeapon's identifier is unique for this component
	// TODO Lok for a better way to do this (currently O(n), same as other operations on Weapons array)
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == NewWeapon->GetIdentifier())
		{
			UE_LOG(LogTemp, Log, TEXT("Not adding new weapon as this component already has a weapon with matching identifier"));
			return false;
		}
	}

	NewWeapon->SetOwningPlayer(OwningCharacter);
	Weapons.Add(NewWeapon);
	return true;
}

void UAWeaponContainerComponent::OnEquipWeaponInput(const FInputActionInstance& Input)
{
	if (Input.GetSourceAction() == EquipRocketAction)
	{
		UE_LOG(LogTemp, Log, TEXT("Equipping Rocket"));
		EquipWeapon(RocketGameplayTag);
	}
	else if (Input.GetSourceAction() == EquipLGAction)
	{
		UE_LOG(LogTemp, Log, TEXT("Equipping LG"));
		EquipWeapon(LGGameplayTag);
	}
	else if (Input.GetSourceAction() == EquipRailAction)
	{
		UE_LOG(LogTemp, Log, TEXT("Equipping Rail"));
		EquipWeapon(RailGameplayTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnEquipWeaponInput couldn't handle input type."));
	}
}

void UAWeaponContainerComponent::OnFireStart()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
}

void UAWeaponContainerComponent::OnFireStop()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

// TODO This is O(n). Could possibly use a Map, though they don't replicate
// Given n is 9 at most, might be ok to leave as is- need to test performance.
void UAWeaponContainerComponent::EquipWeapon(FGameplayTag InIdentifier)
{
	for (AAWeaponBase* Weapon : Weapons)
	{
		if (Weapon && Weapon->GetIdentifier() == InIdentifier)
		{
			Weapon->EquipWeapon();
			EquippedWeapon = Weapon;
		}
	}
}

void UAWeaponContainerComponent::EquipDefaultWeapon()
{
	if (Weapons.IsEmpty())
	{
		return;
	}

	AAWeaponBase* DefaultWeapon = Weapons[0];
	EquipWeapon(DefaultWeapon->GetIdentifier());
}