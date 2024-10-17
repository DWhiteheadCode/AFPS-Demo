#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameplayTagContainer.h"
#include "InputAction.h"

#include "AWeaponContainerComponent.generated.h"

class UInputMappingContext;

class AAWeaponBase;
class AAPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAdded, UAWeaponContainerComponent*, OwningComp, AAWeaponBase*, Weapon);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AFPS_DEMO_API UAWeaponContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAWeaponContainerComponent();

	void EquipDefaultWeapon();

	UFUNCTION(BlueprintCallable)
	TArray<AAWeaponBase*> GetWeapons() const;

	UPROPERTY(BlueprintAssignable)
	FOnWeaponAdded OnWeaponAdded;

protected:
	virtual void BeginPlay() override;

	// OWNER -----------------------------------------------------------------
	UPROPERTY()
	TObjectPtr<AAPlayerCharacter> OwningCharacter;

	// WEAPONS ---------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TArray<TSubclassOf<AAWeaponBase>> DefaultWeapons;
		
	UPROPERTY()
	TArray<TObjectPtr<AAWeaponBase>> Weapons;

	UPROPERTY()
	TObjectPtr<AAWeaponBase> EquippedWeapon;

	UFUNCTION()
	bool InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass);

	UFUNCTION()
	AAWeaponBase* GetWeapon(FGameplayTag WeaponIdentifier) const;

	// WEAPON SWAPPING -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Weapon Swapping")
	float WeaponUnequipDelay = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Weapon Swapping")
	float WeaponEquipDelay = 0.4f;

	UPROPERTY()
	bool bShouldFireOnSwapEnd = false;

	UFUNCTION()
	void StartWeaponSwap();

	UFUNCTION()
	void OnWeaponUnequipDelayEnd();

	UFUNCTION()
	void OnWeaponEquipDelayEnd();

	UPROPERTY()
	bool bIsUnequippingWeapon = false;

	UPROPERTY()
	bool bIsEquippingWeapon = false;

	UPROPERTY()
	TObjectPtr<AAWeaponBase> WeaponToSwapTo = nullptr;

	FTimerHandle TimerHandle_WeaponSwap;

	// INPUT ----------------------------------------------------
	// IMC ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> WeaponInputMappingContext;

	UFUNCTION()
	void SetupWeaponBindings();

	// Fire Action ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UFUNCTION()
	void OnFireStart();

	UFUNCTION()
	void OnFireStop();
	
	// Generic Weapon Swap ----
	UFUNCTION()
	void ProcessSwapInput(FGameplayTag WeaponIdentifier);

	// Equip Rocket ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipRocketAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag RocketGameplayTag;

	UFUNCTION()
	void OnEquipRocketInput();

	// Equip LG ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipLGAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag LGGameplayTag;

	UFUNCTION()
	void OnEquipLGInput();

	// Equip Rail ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipRailAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag RailGameplayTag;

	UFUNCTION()
	void OnEquipRailInput();
};
