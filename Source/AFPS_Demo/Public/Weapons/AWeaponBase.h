#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameplayTagContainer.h"

#include "AWeaponBase.generated.h"

class UStaticMeshComponent;

class AAPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipStateChanged, AAWeaponBase*, Weapon, bool, bIsEquipped);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAmmoChanged, AAWeaponBase*, Weapon, int, NewAmmo, int, OldAmmo, int, MaxAmmo);

UCLASS()
class AFPS_DEMO_API AAWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AAWeaponBase();

	void BeginPlay() override;

	UFUNCTION()
	bool SetOwningPlayer(AAPlayerCharacter* InOwner);

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetIdentifier() const;

	// EQUIP / UNEQUIP --------------------------------------------------------
	UFUNCTION()
	void EquipWeapon();	

	UFUNCTION()
	void UnequipWeapon();

	UFUNCTION(BlueprintCallable)
	bool IsEquipped() const;

	UPROPERTY(BlueprintAssignable)
	FOnEquipStateChanged OnEquipStateChanged;

	// FIRING ------------------------------------------------------------------
	// Called when the user starts firing ("holding the trigger down")
	UFUNCTION()
	void StartFire(); 

	// Called when the user stops firing ("releases the trigger")
	UFUNCTION()
	void StopFire();

	// Called when the weapon actually fires (i.e. each time a bullet/projectile is shot)
	UFUNCTION(BlueprintNativeEvent)
	void Fire();

	UFUNCTION()
	bool CanFire() const;

	UFUNCTION(BlueprintCallable)
	bool IsFiring() const;

	UFUNCTION()
	float GetRemainingFireDelay() const;

	// AMMO -----------------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	int GetAmmo() const;

	UFUNCTION(BlueprintCallable)
	int GetMaxAmmo() const;

	UPROPERTY(BlueprintAssignable)
	FOnAmmoChanged OnAmmoChanged;

protected:
	// COMPONENTS -----------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// IDENTIFIER -----------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FGameplayTag Identifier;

	// OWNER ----------------------------------------------------------------------
	UPROPERTY()
	TObjectPtr<AAPlayerCharacter> OwningPlayer;

	// AMMO -----------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int MaxAmmo = 15;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingAmmo = 10;

	UPROPERTY()
	int Ammo;

	// FIRING ---------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireDelay = 1.f;

	FTimerHandle TimerHandle_FireDelay;

	UPROPERTY()
	float LastFireTime = -1.f;

	UPROPERTY()
	bool bIsFiring = false;

	UFUNCTION()
	void OnFireDelayEnd();

	// EQUIPPED -------------------------------------------------------------------
	UPROPERTY()
	bool bIsEquipped = false;
};
