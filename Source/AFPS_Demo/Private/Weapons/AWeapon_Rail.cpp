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
	if (!CanFire()) // Weapon can't fire (typically out of ammo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to fire [%s], but CanFire() returned false"), *GetNameSafe(this));
		return;
	}

	Super::Fire_Implementation();

	const TArray<UAStackComponent*> HitStackComponents = GetComponentsToDamage();

	for (UAStackComponent* StackComp : HitStackComponents)
	{
		if (StackComp)
		{
			StackComp->ApplyDamage(Damage, OwningPlayer);
		}
	}
}

TArray<UAStackComponent*> AAWeapon_Rail::GetComponentsToDamage() const
{
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] tried to GetComponentsToDamage(), but OwningPlayer was null"), *GetNameSafe(this));
		return TArray<UAStackComponent*>();
	}

	const FVector StartPos = OwningPlayer->GetPawnViewLocation();
	const FRotator FiringDirection = OwningPlayer->GetControlRotation();

	const FVector EndPos = StartPos + (FiringDirection.Vector() * Range);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningPlayer);
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, StartPos, EndPos, TraceChannel, QueryParams);

	TArray<UAStackComponent*> ComponentsToDamage;

	for (const FHitResult Hit : HitResults)
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
