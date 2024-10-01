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

bool AAWeaponBase::SetOwningPlayer(AAPlayerCharacter* InOwner)
{
	if (!InOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't set weapon owner to null"));
		return false;
	}

	OwningPlayer = InOwner;
	AttachToComponent(OwningPlayer->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	return true;
}

void AAWeaponBase::EquipWeapon()
{
	MeshComp->SetVisibility(true, true);
		
	// TODO Attach firing bind
}



bool AAWeaponBase::UnequipWeapon()
{
	MeshComp->SetVisibility(false, true);

	// TODO Detach firing bind

	return true;
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}

