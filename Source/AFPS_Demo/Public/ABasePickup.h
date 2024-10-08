#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABasePickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class AFPS_DEMO_API AABasePickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AABasePickup();

	void PostInitializeComponents() override;

protected:
	// COMPONENTS ---------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	// COLLISION ----------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Collision")
	float CollisionRadius = 50.f;

	UFUNCTION(BlueprintNativeEvent)
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// PICKUP FUNCTIONALITY ------------------------------------------
	UFUNCTION()
	void StartCooldown();

	UFUNCTION()
	void OnCooldownEnd();

	UFUNCTION()
	void UpdatePickupState();

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnCooldown = false;

	UPROPERTY(EditAnywhere, Category="Cooldown")
	float CooldownDuration = 30.f;

	FTimerHandle TimerHandle_Cooldown;
};
