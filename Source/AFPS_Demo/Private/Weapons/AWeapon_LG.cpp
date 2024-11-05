#include "Weapons/AWeapon_LG.h"

#include "APlayerCharacter.h"
#include "AStackComponent.h"

AAWeapon_LG::AAWeapon_LG()
{
	MaxAmmo = 150;
	StartingAmmo = 100;
	FireDelay = 0.055f;
}

void AAWeapon_LG::Fire_Implementation()
{
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}

	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to fire [%s], but OwningPlayer was null"), *GetNameSafe(this));
		return;
	}

	Super::Fire_Implementation();
	
	const FVector StartPos = OwningPlayer->GetPawnViewLocation();
	const FRotator FiringDirection = OwningPlayer->GetControlRotation();

	const FVector EndPos = StartPos + (FiringDirection.Vector() * Range);

	if (HasAuthority())
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwningPlayer);
		QueryParams.AddIgnoredActor(this);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, TraceChannel, QueryParams);

		const AActor* HitActor = HitResult.GetActor();
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
	else // Show client prediction of the shot
	{
		DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::White, false, FireDelay, 0, 1.f);
	}

	
}
