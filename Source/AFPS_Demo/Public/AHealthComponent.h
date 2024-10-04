// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
	FOnHealthChanged,
	UAHealthComponent*, OwningComp,
	AActor*, InstigatorActor,
	int, NewHealth,
	int, DeltaHealth,
	int, NewArmour,
	int, DeltaArmour
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AFPS_DEMO_API UAHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAHealthComponent();

	UFUNCTION()
	bool ApplyDamage(const int Amount, AActor* InstigatorActor);

	UFUNCTION()
	bool AddHealth(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor);

	UFUNCTION()
	bool AddArmour(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor);

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

protected:
	UPROPERTY()
	int Health = 125;

	UPROPERTY()
	int Armour = 50;

	UPROPERTY(EditAnywhere, Category = "Health")
	float ArmourEfficiency = 0.67f; // Ratio of damage dealt that is applied to armour
	
	UPROPERTY(EditAnywhere, Category = "Health")
	int BaseHealthMax = 100;

	UPROPERTY(EditAnywhere, Category = "Health")
	int OverHealthMax = 125; // Max health, including base-health and overhealth

	UPROPERTY(EditAnywhere, Category = "Health")
	int BaseArmourMax = 75;

	UPROPERTY(EditAnywhere, Category = "Health")
	int OverArmourMax = 100; // Max armour, including base-armour and overarmour
		
};
