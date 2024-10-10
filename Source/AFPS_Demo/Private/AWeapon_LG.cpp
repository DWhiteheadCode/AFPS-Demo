#include "AWeapon_LG.h"

#include "APlayerCharacter.h"
#include "AStackComponent.h"

AAWeapon_LG::AAWeapon_LG()
{
	MaxAmmo = 150;
	StartingAmmo = 100;
	FireDelay = 0.2f;
}

void AAWeapon_LG::Fire_Implementation()
{
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire LG, but CanFire() returned false"));
		return;
	}

	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to fire LG, but OwningPlayer was null"));
		return;
	}

	Super::Fire_Implementation();
	
	FVector StartPos = OwningPlayer->GetPawnViewLocation();
	FRotator FiringDirection = OwningPlayer->GetControlRotation();

	FVector EndPos = StartPos + (FiringDirection.Vector() * Range);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningPlayer);
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, TraceChannel, QueryParams);

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
		DrawDebugLine(GetWorld(), StartPos, HitResult.ImpactPoint, FColor::Red, false, FireDelay, 0, 1.f);
	}
	else
	{
		DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Blue, false, FireDelay, 0, 1.f);
	}
}
