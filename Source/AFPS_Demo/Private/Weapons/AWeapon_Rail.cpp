#include "Weapons/AWeapon_Rail.h"

#include "AStackComponent.h"
#include "APlayerCharacter.h"

AAWeapon_Rail::AAWeapon_Rail()
{
	MaxAmmo = 10;
	StartingAmmo = 10;
	FireDelay = 1.7f;
}

void AAWeapon_Rail::Fire_Implementation()
{
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Rail [%s] tried to fire, but OwningPlayer was null."), *GetNameSafe(this));
		return;
	}

	if (!CanFire()) // WeaponContainerComponent should have checked CanFire() before calling Fire().
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}

	Super::Fire_Implementation();

	const FVector StartPos = OwningPlayer->GetPawnViewLocation();
	const FRotator FiringDirection = OwningPlayer->GetControlRotation();

	const FVector EndPos = StartPos + (FiringDirection.Vector() * Range);

	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::White, false, FireDelay, 0, 1.f);

	if (HasAuthority())
	{
		const TArray<UAStackComponent*> HitStackComponents = GetStackComponentsInLine(StartPos, EndPos);

		for (UAStackComponent* StackComp : HitStackComponents)
		{
			if (StackComp)
			{
				StackComp->ApplyDamage(Damage, OwningPlayer);
			}
		}
	}
}

TArray<UAStackComponent*> AAWeapon_Rail::GetStackComponentsInLine(const FVector StartPos, const FVector EndPos) const
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client Rail attempted to get ComponentsToDamage. Returning empty array."));
		return TArray<UAStackComponent*>();
	}

	FCollisionQueryParams QueryParams;
	
	QueryParams.AddIgnoredActor(this);

	if (OwningPlayer)
	{
		QueryParams.AddIgnoredActor(OwningPlayer);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Rail [%s] tried to GetComponentsToDamage(), but OwningPlayer was null so they can't be ignored"), *GetNameSafe(this));
	}

	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, StartPos, EndPos, TraceChannel, QueryParams);

	TArray<UAStackComponent*> ComponentsToDamage;

	for (const FHitResult& Hit : HitResults)
	{
		FColor DebugColor = FColor::Blue;
		
		const AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UAStackComponent* StackComp = Cast<UAStackComponent>(HitActor->GetComponentByClass(UAStackComponent::StaticClass()));

			if (StackComp)
			{
				ComponentsToDamage.Add(StackComp);
				DebugColor = FColor::Red;
			}
		}

		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 16, DebugColor, false, 3.f, 0, 1.f);
	}
	
	return ComponentsToDamage;
}
