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

	UFUNCTION()
	bool SetOwningPlayer(AAPlayerCharacter* InOwner);

	UFUNCTION()
	void EquipWeapon();	

	UFUNCTION()
	void UnequipWeapon();

	UFUNCTION(BlueprintNativeEvent)
	void StartFire();

	UFUNCTION(BlueprintNativeEvent)
	void StopFire();

	UFUNCTION()
	FGameplayTag GetIdentifier() const;

protected:
	// COMPONENTS -----------------------
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// PROPERTIES -----------------------
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FGameplayTag Identifier;

	UPROPERTY()
	TObjectPtr<AAPlayerCharacter> OwningPlayer;



};
