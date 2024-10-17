#include "Projectiles/AProjectile_Rocket.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "AStackComponent.h"

AAProjectile_Rocket::AAProjectile_Rocket()
{
	CollisionSphereComp = CreateDefaultSubobject<USphereComponent>("CollisionSphereComp");
	CollisionSphereComp->SetSphereRadius(CollisionRadius);
	CollisionSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphereComp->SetCollisionProfileName("Projectile");
	CollisionSphereComp->SetGenerateOverlapEvents(true);
	
	RootComponent = CollisionSphereComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMoveComp");
	ProjectileMovementComp->ProjectileGravityScale = 0.f;
	ProjectileMovementComp->InitialSpeed = 1000.f;
}

void AAProjectile_Rocket::PostInitializeComponents()
{
	Super::PostInitializeComponents();

    CollisionSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAProjectile_Rocket::OnBeginOverlap);
}

void AAProjectile_Rocket::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Can't direct-hit self
	if (OtherActor == GetInstigator())
	{
		return;
	}

    Detonate();
}

void AAProjectile_Rocket::Detonate()
{
	DrawDebugSphere(GetWorld(), GetActorLocation(), CloseFalloffRange, 16, FColor::White, false, 5.f, 0, 1.f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), FarFalloffRange, 16, FColor::White, false, 5.f, 0, 1.f);

    TArray<AActor*> NearbyActors = GetActorsInExplosionRadius();
 
    for (AActor* Actor : NearbyActors)
    {
		if (!Actor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Nullptr Actor returned from GetNearbyActors()"));
			continue;
		}

		UAStackComponent* StackComp = Cast<UAStackComponent>(Actor->GetComponentByClass(UAStackComponent::StaticClass()));
        if (StackComp)
        {
            const int Damage = CalculateDamage(Actor);

            if (Damage > 0)
            {
                StackComp->ApplyDamage(Damage, GetInstigator());
            }
        }
    }

	// Destroy self
	SetActorEnableCollision(false);
	MeshComp->SetVisibility(false);
	SetLifeSpan(2.f);
}

TArray<AActor*> AAProjectile_Rocket::GetActorsInExplosionRadius() const
{
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    const TArray<AActor*> ActorsToIgnore;
    TArray<AActor*> NearbyActors;

    UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), FarFalloffRange, ObjectTypes, nullptr, ActorsToIgnore, NearbyActors);

    return NearbyActors;
}

int AAProjectile_Rocket::CalculateDamage(AActor* ActorToDamage) const
{
	if (FarFalloffRange <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("Rocket FarFalloffRange must be > 0. Returning 0 damage."));
		return 0;
	}

	if (CloseFalloffRange <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("Rocket CloseFalloffRange must be > 0. Returning 0 damage."));
		return 0;
	}

	if (!ActorToDamage)
	{
		UE_LOG(LogTemp, Error, TEXT("Rocket tried to calculate damage for nullptr ActorToDamage"));
		return 0;
	}

	// If, in the future, it's considered valid for more damage to be dealt the further away the target is 
	// from the rocket, then make sure (FarFalloffRange == CloseFalloffRange) is covered to prevent
	// a divide-by-zero in the Lerp function below.
	if (FarFalloffRange <= CloseFalloffRange)
	{
		UE_LOG(LogTemp, Error, TEXT("Rocket FarFalloffRange must be > CloseFalloffRange. Returning 0 damage."));
		return 0;
	}

	// Can't deal damage through walls
	if (IsDamagePathBlocked(ActorToDamage))
	{
		return 0; 
	}

	// This is not ideal as the actual distance we want is the closest distance between the rocket
	// and any point on ActorToDamage's hitbox that has LOS to the rocket.
	//
	// TODO - Leaving it like this for now, but will look for a better solution later.
	const float Distance = FVector::Distance(GetActorLocation(), ActorToDamage->GetActorLocation());

	if (Distance > FarFalloffRange)
	{
		// This is currently common if part of ActorToDamage is inside FarFalloffRange, but their GetActorLocation()
		// is outside it. See comment on "Distance" above.
		UE_LOG(LogTemp, Warning, TEXT("Rocket tried to damage Actor outside MinDamage_Range."));
		return 0;
	}

	if (Distance <= CloseFalloffRange)
	{
		return MaxDamage;
	}

	// Deal MaxDamage at CloseFalloffRange, scaling linearly down to MinDamage if Distance == FarFalloffRange.
	// Note: The case where the denominator == 0 is covered by the above guard conditions.
	const int Damage = FMath::Lerp(MaxDamage, MinDamage, ((Distance - CloseFalloffRange) / (FarFalloffRange - CloseFalloffRange)));

	//UE_LOG(LogTemp, Log, TEXT("Distance to rocket: %f"), Distance);
	//UE_LOG(LogTemp, Log, TEXT("Damage from rocket: %i"), Damage);

	return Damage;
}

// This approach is not ideal as damage will be blocked if the GetActorLocation() of ActorToDamage
// is blocked, regardless of if there is valid LOS to another part of the Actor.
//
// TODO - Leaving it like this for now, but will look for a better solution later.
bool AAProjectile_Rocket::IsDamagePathBlocked(AActor* ActorToDamage) const
{
	if (!ActorToDamage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rocket tried to check IsDamagePathBlocked for nullptr ActorToDamage. Returning true."));
		return true;
	}

	// Perform a line trace from rocket -> ActorToDamage
	// If the trace hits something (that's not ActorToDamage), then LOS is considered blocked.
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ActorToDamage);

	const FVector StartLocation = GetActorLocation();
	const FVector EndLocation = ActorToDamage->GetActorLocation();

	FHitResult HitResult;

	return GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, SplashDamageTraceChannel, Params);
}