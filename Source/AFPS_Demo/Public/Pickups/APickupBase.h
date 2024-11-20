#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APickupBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;

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
	
	// PICKUP FUNCTIONALITY ------------------------------------------
	UPROPERTY(EditAnywhere, Category="Cooldown")
	bool bRespawns = true;

	UPROPERTY(EditAnywhere, Category = "Cooldown")
	float CooldownDuration = 30.f;

	UPROPERTY(ReplicatedUsing="OnRep_CooldownStateChanged", BlueprintReadOnly)
	bool bIsOnCooldown = false;

	UFUNCTION()
	void StartCooldown();

	UFUNCTION()
	void OnCooldownEnd();

	UFUNCTION()
	void UpdatePickupState();

	UFUNCTION()
	void OnRep_CooldownStateChanged();

	FTimerHandle TimerHandle_Cooldown;
};
