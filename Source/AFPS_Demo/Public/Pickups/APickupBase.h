#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APickupBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USoundCue;

UCLASS()
class AFPS_DEMO_API AAPickupBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AAPickupBase();

	void PostInitializeComponents() override;

protected:
	// COMPONENTS ---------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphereComp;

	// COLLISION ----------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Collision")
	float CollisionRadius = 50.f;

	UFUNCTION(BlueprintNativeEvent)
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// SOUND ---------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundCue> PickupSound;

	// PICKUP FUNCTIONALITY ------------------------------------------
	UFUNCTION()
	virtual bool CanPickup(AActor* OtherActor);

	UFUNCTION()
	virtual void Pickup(AActor* OtherActor);
	
	UPROPERTY(EditAnywhere, Category="Cooldown")
	bool bRespawns = true;

	UPROPERTY(EditAnywhere, Category = "Cooldown")
	float CooldownDuration = 30.f;

	UPROPERTY(ReplicatedUsing="OnRep_IsActive", BlueprintReadOnly)
	bool bIsActive = true;

	UFUNCTION()
	void StartCooldown();

	UFUNCTION()
	void OnCooldownEnd();

	UFUNCTION()
	void OnRep_IsActive();

	FTimerHandle TimerHandle_Cooldown;
};
