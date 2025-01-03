#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameplayTagContainer.h"

#include "AWeaponBase.generated.h"

class UStaticMeshComponent;
class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIsEquippedChanged, AAWeaponBase*, Weapon, bool, bIsEquipped);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIsEquippableChanged, AAWeaponBase*, Weapon, bool, bIsEquippable);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChanged, AAWeaponBase*, Weapon, int, NewAmmo, int, OldAmmo);

UCLASS()
class AFPS_DEMO_API AAWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AAWeaponBase();

	void BeginPlay() override;

	UFUNCTION()
	bool SetOwningCharacter(ACharacter* InOwner);

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
	bool IsEquippable() const;

	UFUNCTION()
	void SetIsEquippable(bool bInEquippable);

	UFUNCTION(BlueprintCallable)
	bool IsEquipped() const;

	UPROPERTY(BlueprintAssignable)
	FOnIsEquippedChanged OnIsEquippedChanged;

	UPROPERTY(BlueprintAssignable)
	FOnIsEquippableChanged OnIsEquippableChanged;

	// FIRING ------------------------------------------------------------------
	UFUNCTION()
	void SetIsTriggerHeld(const bool bInTriggerHeld); 

	// Called when the weapon actually fires (i.e. each time a bullet/projectile is shot)
	UFUNCTION()
	virtual void Fire();

	UFUNCTION()
	bool CanFire() const;

	UFUNCTION(BlueprintCallable)
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable)
	bool IsTriggerHeld() const;

	UFUNCTION()
	float GetRemainingFireDelay() const;

	// AMMO -----------------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	int GetAmmo() const;

	UFUNCTION()
	int GetStartingAmmo() const;

	UFUNCTION(BlueprintCallable)
	int GetMaxAmmo() const;

	UFUNCTION()
	void AddAmmo(const int InAmount);

	UFUNCTION()
	void SetAmmo(const int InAmount);

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
	TObjectPtr<ACharacter> OwningPlayer;

	UFUNCTION()
	void OnRep_OwningPlayer();

	// AMMO -----------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int MaxAmmo = 15;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	bool bInfiniteAmmo = false;

	UPROPERTY(ReplicatedUsing="OnRep_Ammo")
	int Ammo = 0;

	UFUNCTION()
	void OnRep_Ammo(int OldAmmo);

	// FIRING ---------------------------------------------------------------------
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayNonLoopFireSound();
	
	UFUNCTION()
	void StartInitialFireDelay();

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireDelay = 1.f;

	FTimerHandle TimerHandle_FireDelay;

	UPROPERTY(Replicated)
	float LastFireTime = -1.f;

	UPROPERTY(ReplicatedUsing="OnRep_IsFiring")
	bool bIsFiring = false;

	UFUNCTION()
	void OnRep_IsFiring();

	UPROPERTY(Replicated)
	bool bIsTriggerHeld = false;

	UFUNCTION()
	void OnInitialFireDelayEnd();

	// EQUIPPED -------------------------------------------------------------------
	UPROPERTY(ReplicatedUsing="OnRep_IsEquippable")
	bool bIsEquippable = false;
	
	UFUNCTION()
	void OnRep_IsEquippable();

	UPROPERTY(ReplicatedUsing="OnRep_IsEquipped")
	bool bIsEquipped = false;

	UFUNCTION()
	void OnRep_IsEquipped();

	// AUDIO -----------------------------------------------------------------------
	UFUNCTION()
	void StartFiringAudioLoop();

	UFUNCTION()
	void StopFiringAudioLoop();

	UFUNCTION()
	bool FiringAudioIsLoop();

};
