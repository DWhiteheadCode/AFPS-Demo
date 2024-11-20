#include "Weapons/AWeapon_LG.h"

#include "APlayerCharacter.h"
#include "AStackComponent.h"

AAWeapon_LG::AAWeapon_LG()
{
	MaxAmmo = 150;
	StartingAmmo = 100;
	FireDelay = 0.055f;
}

void AAWeapon_LG::Fire()
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

	Super::Fire();

	if ( (OwningPlayer && OwningPlayer->IsLocallyControlled()) || HasAuthority() )
	{
		FHitResult HitResult = PerformTrace();

		if (HasAuthority())
		{
			const AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				UAStackComponent* StackComp = Cast<UAStackComponent>(HitActor->GetComponentByClass(UAStackComponent::StaticClass()));

				if (StackComp)
				{
					StackComp->ApplyDamage(Damage, OwningPlayer);
				}
			}
		}
	}	
}

FHitResult AAWeapon_LG::PerformTrace()
{
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LG [%s] tried to perform trace, but OwningPlayer was null."), *GetNameSafe(this));
		return FHitResult();
	}

	const FVector StartPos = OwningPlayer->GetPawnViewLocation();
	const FRotator FiringDirection = OwningPlayer->GetControlRotation();

	FVector EndPos = StartPos + (FiringDirection.Vector() * Range);

	FCollisionQueryParams QueryParams;

	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwningPlayer);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, TraceChannel, QueryParams);

	// Update EndPos to draw debug line
	if (HitResult.bBlockingHit)
	{
		EndPos = HitResult.ImpactPoint;
	}

	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Blue, false, FireDelay, 0, 1.f);

	if (HasAuthority())
	{
		MulticastDrawBeam(StartPos, EndPos);
	}

	return HitResult;
}

void AAWeapon_LG::MulticastDrawBeam_Implementation(const FVector Start, const FVector End)
{
	// Local player already drew beam at time of firing
	if (OwningPlayer && OwningPlayer->IsLocallyControlled())
	{
		return;
	}

	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, FireDelay, 0, 1.f);
}
