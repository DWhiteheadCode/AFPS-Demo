#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameplayTagContainer.h"

#include "AWeaponBase.generated.h"

class UStaticMeshComponent;

class AAPlayerCharacter;

UCLASS()
class AFPS_DEMO_API AAWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AAWeaponBase();

	void BeginPlay() override;

	UFUNCTION()
	bool SetOwningPlayer(AAPlayerCharacter* InOwner);

	UFUNCTION()
	void EquipWeapon();	

	UFUNCTION()
	void UnequipWeapon();

	// Called when the user starts firing ("holding the trigger down")
	UFUNCTION()
	void StartFire(); 

	// Called when the user stops firing ("releases the trigger")
	UFUNCTION()
	void StopFire();

	// Called when the weapoon actually fires (i.e. each time a bullet/projectile is shot)
	UFUNCTION(BlueprintNativeEvent)
	void Fire();

	UFUNCTION()
	bool CanFire() const;

	UFUNCTION(BlueprintCallable)
	bool IsFiring() const;

	UFUNCTION()
	FGameplayTag GetIdentifier() const;

protected:
	UFUNCTION()
	void OnFireDelayEnd();
	
	// COMPONENTS -----------------------
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// PROPERTIES -----------------------
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FGameplayTag Identifier;

	UPROPERTY()
	TObjectPtr<AAPlayerCharacter> OwningPlayer;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int MaxAmmo = 15;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingAmmo = 10;

	UPROPERTY()
	int Ammo;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireDelay = 1.f;

	FTimerHandle TimerHandle_FireDelay;

	UPROPERTY()
	float LastFireTime = -1.f;

	UPROPERTY()
	bool bIsFiring = false;
};
