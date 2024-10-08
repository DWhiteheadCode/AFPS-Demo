#include "AWeapon_Rail.h"

#include "AStackComponent.h"
#include "APlayerCharacter.h"

AAWeapon_Rail::AAWeapon_Rail()
{
	FireDelay = 1.7f;
}

void AAWeapon_Rail::Fire_Implementation()
{
	if (!CanFire()) // Weapon can't fire (typically out of ammo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire Rail, but CanFire() returned false"));
		return;
	}

	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to fire rail, but didn't have OwningPlayer"));
		return;
	}

	Super::Fire_Implementation();

	FVector StartPos = OwningPlayer->GetPawnViewLocation();
	FRotator FiringDirection = OwningPlayer->GetControlRotation();

	FVector EndPos = StartPos + (FiringDirection.Vector() * Range);	

	FCollisionObjectQueryParams Params;
	Params.AddObjectTypesToQuery(ECC_Pawn);
	Params.AddObjectTypesToQuery(ECC_WorldStatic);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningPlayer);
	QueryParams.AddIgnoredActor(this);
	
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByObjectType(HitResult, StartPos, EndPos, Params, QueryParams);

	AActor* HitActor = HitResult.GetActor();
	if (HitActor)
	{
		UAStackComponent* StackComp = Cast<UAStackComponent>(HitActor->GetComponentByClass(UAStackComponent::StaticClass()));

		if (StackComp)
		{
			StackComp->ApplyDamage(Damage, OwningPlayer); 
		}
	}

	if (HitResult.bBlockingHit)
	{
		DrawDebugLine(GetWorld(), StartPos, HitResult.ImpactPoint, FColor::Blue, false, 2.f, 0, 2.f);
	}
	else
	{
		DrawDebugLine(GetWorld(), StartPos, HitResult.TraceEnd, FColor::Blue, false, 2.f, 0, 2.f);
	}

}
