#include "Weapons/AWeapon_Rail.h"

#include "AStackComponent.h"
#include "APlayerCharacter.h"

AAWeapon_Rail::AAWeapon_Rail()
{
	MaxAmmo = 10;
	StartingAmmo = 10;
	FireDelay = 1.7f;
}

void AAWeapon_Rail::Fire()
{
	if (!CanFire()) // WeaponContainerComponent should have checked CanFire() before calling Fire().
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}

	Super::Fire();

	const TArray<FHitResult> HitResults = PerformTrace(); // Perform trace even on clients to show rail trail

	if (HasAuthority())
	{
		const TArray<UAStackComponent*> HitStackComponents = GetStackComponentsFromHitResults(HitResults);

		for (UAStackComponent* StackComp : HitStackComponents)
		{
			if (StackComp)
			{
				StackComp->ApplyDamage(Damage, OwningPlayer);
			}
		}
	}
}

TArray<FHitResult> AAWeapon_Rail::PerformTrace()
{
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Rail [%s] tried to perform trace, but OwningPlayer was null."), *GetNameSafe(this));
		return TArray<FHitResult>();
	}

	const FVector StartPos = OwningPlayer->GetPawnViewLocation();
	const FRotator FiringDirection = OwningPlayer->GetControlRotation();

	FVector EndPos = StartPos + (FiringDirection.Vector() * Range);

	FCollisionQueryParams QueryParams;

	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwningPlayer);

	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, StartPos, EndPos, TraceChannel, QueryParams);

	// Update EndPos to draw debug line
	if (HitResults.Last().bBlockingHit)
	{
		EndPos = HitResults.Last().ImpactPoint;
	}
	
	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Green, false, FireDelay, 0, 1.f);

	return HitResults;
}

TArray<UAStackComponent*> AAWeapon_Rail::GetStackComponentsFromHitResults(const TArray<FHitResult> HitResults) const
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to GetStackComponentsFromHitResults() in [%s]. Returning empty array."), *GetNameSafe(this));
		return TArray<UAStackComponent*>();
	}

	TArray<UAStackComponent*> StackComponents;

	for (const FHitResult& Hit : HitResults)
	{
		FColor DebugColor = FColor::Blue;

		const AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UAStackComponent* StackComp = Cast<UAStackComponent>(HitActor->GetComponentByClass(UAStackComponent::StaticClass()));

			if (StackComp)
			{
				StackComponents.Add(StackComp);
				DebugColor = FColor::Red;
			}
		}

		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 16, DebugColor, false, 3.f, 0, 1.f);
	}

	return StackComponents;
}
