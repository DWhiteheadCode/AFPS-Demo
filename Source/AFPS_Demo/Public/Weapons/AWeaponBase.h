#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameplayTagContainer.h"

#include "AWeaponBase.generated.h"

class UStaticMeshComponent;
class USoundCue;

class AAPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipStateChanged, AAWeaponBase*, Weapon, bool, bIsEquipped);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChanged, AAWeaponBase*, Weapon, int, NewAmmo, int, OldAmmo);

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
	bool IsLocallyOwned() const;

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
	UFUNCTION()
	virtual void Fire();

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

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AmbientAudioComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UAudioComponent> FiringAudioComp;

	// IDENTIFIER -----------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Replicated, Category="Weapon")
	FGameplayTag Identifier;

	// OWNER ----------------------------------------------------------------------
	UPROPERTY(ReplicatedUsing="OnRep_OwningPlayer")
	TObjectPtr<AAPlayerCharacter> OwningPlayer;

	UFUNCTION()
	void OnRep_OwningPlayer();

	// AMMO -----------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int MaxAmmo = 15;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingAmmo = 10;

	UPROPERTY(Replicated)
	int Ammo = 0;

	// Only used for UI changes so could maybe be unreliable, though packet loss could 
	// result in UI showing the user has ammo when they don't, which would be jarring.
	UFUNCTION(Client, Reliable) 
	void ClientAmmoChanged(const int NewAmmo, const int OldAmmo);

	// FIRING ---------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireDelay = 1.f;

	FTimerHandle TimerHandle_FireDelay;

	UPROPERTY(Replicated)
	float LastFireTime = -1.f;

	UPROPERTY(ReplicatedUsing="OnRep_IsFiring")
	bool bIsFiring = false;

	UFUNCTION()
	void OnRep_IsFiring();

	UFUNCTION()
	void OnFireDelayEnd();

	// EQUIPPED -------------------------------------------------------------------
	UPROPERTY(ReplicatedUsing="OnRep_IsEquippedChanged")
	bool bIsEquipped = false;

	UFUNCTION()
	void OnRep_IsEquippedChanged();
};
