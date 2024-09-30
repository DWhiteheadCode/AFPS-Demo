#include "AWeaponBase.h"

#include "Components/StaticMeshComponent.h"

#include "APlayerCharacter.h"

AAWeaponBase::AAWeaponBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);
}

bool AAWeaponBase::EquipWeapon(AAPlayerCharacter* InOwner)
{
	if (!InOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to EquipWeapon(), but OwningPlayer was null"));
		return false;
	}

	OwningPlayer = InOwner;
	AttachToComponent(OwningPlayer->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// TODO Attach firing bind

	// TODO Update model/ visibility

	return true;
}

bool AAWeaponBase::UnequipWeapon()
{


	// TODO Detach firing bind

	// TODO Update model/ visibility
	

	return true;
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}

