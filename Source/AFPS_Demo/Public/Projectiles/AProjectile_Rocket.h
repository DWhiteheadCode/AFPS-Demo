#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AProjectile_Rocket.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;

class UAStackComponent;

UCLASS()
class AFPS_DEMO_API AAProjectile_Rocket : public AActor
{
	GENERATED_BODY()
	
public:	
	AAProjectile_Rocket();

	void PostInitializeComponents() override;

protected:
	// COMPONENTS ----------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

	// DETONATION ----------------------------------------------------------
	UFUNCTION()
	void Detonate();

	// COLLISION -----------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Collision")
	float CollisionRadius = 16.f; 

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> SplashDamageTraceChannel;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// DAMAGE ---------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category="Damage")
	int MinDamage = 10;

	UPROPERTY(EditAnywhere, Category = "Damage")
	int MaxDamage = 100; 

	UPROPERTY(EditAnywhere, Category = "Damage")
	float CloseFalloffRange = 10.f; // Actors closer than this will take MaxDamage

	UPROPERTY(EditAnywhere, Category = "Damage")
	float FarFalloffRange = 100.f; // Range where MinDamage will be dealt. Also max range to deal any damage.

	UFUNCTION()
	int CalculateDamage(const AActor* ActorToDamage) const;

	// HELPER FUNCTIONS ------------------------------------------------------
	UFUNCTION()
	TArray<AActor*> GetActorsInExplosionRadius() const; 

	UFUNCTION()
	bool IsDamagePathBlocked(const AActor* ActorToDamage) const;
};
