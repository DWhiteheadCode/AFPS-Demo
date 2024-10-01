#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameplayTagContainer.h"

#include "AWeaponContainerComponent.generated.h"

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
};
