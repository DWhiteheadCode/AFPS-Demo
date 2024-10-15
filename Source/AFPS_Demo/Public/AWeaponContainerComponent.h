#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameplayTagContainer.h"
#include "InputAction.h"

#include "AWeaponContainerComponent.generated.h"

class UInputMappingContext;

class AAWeaponBase;
class AAPlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AFPS_DEMO_API UAWeaponContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAWeaponContainerComponent();

	void EquipWeapon(FGameplayTag InIdentifier);

	void EquipDefaultWeapon();

	UFUNCTION(BlueprintCallable)
	TArray<AAWeaponBase*> GetWeapons() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TArray<TSubclassOf<AAWeaponBase>> DefaultWeapons;
		
	UPROPERTY()
	TArray<TObjectPtr<AAWeaponBase>> Weapons;

	UPROPERTY()
	TObjectPtr<AAPlayerCharacter> OwningCharacter;

	UPROPERTY()
	TObjectPtr<AAWeaponBase> EquippedWeapon;

	UFUNCTION()
	bool InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass);

	// INPUT ----------------------------------------------------
	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> WeaponInputMappingContext;

	// Fire Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UFUNCTION()
	void OnFireStart();

	UFUNCTION()
	void OnFireStop();

	// ALL WEAPON SWAP INPUTS ---------------
	// Generic EquipWeaponInput 
	UFUNCTION()
	void OnEquipWeaponInput(const FInputActionInstance& Input);

	// Rocket
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipRocketAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag RocketGameplayTag;

	// LG
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipLGAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag LGGameplayTag;

	// Rail
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipRailAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag RailGameplayTag;
};
