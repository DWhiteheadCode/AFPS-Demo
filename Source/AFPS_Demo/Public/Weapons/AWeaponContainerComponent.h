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

UENUM(BlueprintType)
enum WeaponEquipState
{
	NOT_EQUIPPED		 UMETA(DisplayName = "NOT EQUIPPED"),
	WAITING_TO_UNEQUIP   UMETA(DisplayName = "WAITING TO UNEQUIP"),
	UNEQUIPPING			 UMETA(DisplayName = "UNEQUIPPING"),
	EQUIPPING			 UMETA(DisplayName = "EQUIPPING"),
	READY				 UMETA(DisplayName = "READY")
};

USTRUCT()
struct FWeaponSwapRepData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<WeaponEquipState> WeaponEquipState = WeaponEquipState::NOT_EQUIPPED;

	UPROPERTY()
	bool bShouldFireOnSwapEnd = false;

	UPROPERTY()
	TObjectPtr<AAWeaponBase> WeaponToSwapTo = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AFPS_DEMO_API UAWeaponContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAWeaponContainerComponent();

	void EquipDefaultWeapon();

	UPROPERTY(BlueprintAssignable)
	FOnWeaponAdded OnWeaponAdded;

	UFUNCTION(Client, Unreliable)
	void ClientOnWeaponAdded(AAWeaponBase* NewWeapon);

protected:
	virtual void BeginPlay() override;

	// OWNER -----------------------------------------------------------------
	UPROPERTY()
	TObjectPtr<AAPlayerCharacter> OwningCharacter;

	// WEAPONS ---------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TArray<TSubclassOf<AAWeaponBase>> DefaultWeapons;
		
	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<TObjectPtr<AAWeaponBase>> Weapons;

	UPROPERTY(Replicated)
	TObjectPtr<AAWeaponBase> EquippedWeapon;

	UFUNCTION()
	bool InstantiateWeapon(TSubclassOf<AAWeaponBase> WeaponClass);

	UFUNCTION()
	AAWeaponBase* GetWeapon(const FGameplayTag WeaponIdentifier) const;

	// WEAPON SWAPPING -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Weapon Swapping")
	float WeaponUnequipDelaySeconds = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Weapon Swapping")
	float WeaponEquipDelaySeconds = 0.4f;

	UFUNCTION()
	void StartWeaponSwap();

	UFUNCTION()
	void OnWeaponUnequipDelayEnd();

	UFUNCTION()
	void OnWeaponEquipDelayEnd();

	UPROPERTY(ReplicatedUsing="OnRep_WeaponSwap")
	FWeaponSwapRepData RepData_WeaponSwap;

	UFUNCTION()
	void OnRep_WeaponSwap();

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
	void OnFireStartInput();

	UFUNCTION(Server, Reliable)
	void ServerOnFireStartInput();

	UFUNCTION()
	void OnFireStopInput();

	UFUNCTION(Server, Reliable)
	void ServerOnFireStopInput();
	
	// Generic Weapon Swap ----
	UFUNCTION()
	void ProcessSwapInput(FGameplayTag WeaponIdentifier);

	UFUNCTION(Server, Reliable)
	void ServerProcessSwapInput(FGameplayTag WeaponIdentifier);

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
