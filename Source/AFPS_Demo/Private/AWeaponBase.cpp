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
	AttachToComponent(OwningPlayer->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_r"));

	return true;
}

void AAWeaponBase::EquipWeapon()
{
	MeshComp->SetVisibility(true, true);
		
	// TODO Attach firing bind
}

void AAWeaponBase::UnequipWeapon()
{
	MeshComp->SetVisibility(false, true);

	// TODO Detach firing bind

}

void AAWeaponBase::Fire_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Firing base weapon"));
}

FGameplayTag AAWeaponBase::GetIdentifier() const
{
	return Identifier;
}