// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AStackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
	FOnStackChanged,
	UAStackComponent*, OwningComp,
	AActor*, InstigatorActor,
	int, NewHealth,
	int, DeltaHealth,
	int, NewArmour,
	int, DeltaArmour
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AFPS_DEMO_API UAStackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAStackComponent();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	bool ApplyDamage(const int Amount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable)
	bool AddHealth(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable)
	bool AddArmour(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor);

	UPROPERTY(BlueprintAssignable)
	FOnStackChanged OnStackChanged;

	UFUNCTION(BlueprintCallable)
	int GetHealth() const;

	UFUNCTION(BlueprintCallable)
	int GetArmour() const;


protected:
	// HEALTH -------------------------
	UPROPERTY()
	int Health = 125;

	UPROPERTY(EditAnywhere, Category = "Health")
	int StartingHealth = 125;

	UPROPERTY(EditAnywhere, Category = "Health")
	int BaseHealthMax = 100;

	UPROPERTY(EditAnywhere, Category = "Health")
	int OverHealthMax = 175; // Max health, including base-health and overhealth

	UPROPERTY(EditAnywhere, Category = "Health")
	float OverHealthDecayInterval = 1.f; // Time (seconds) between over-health decay ticks. 

	UPROPERTY(EditAnywhere, Category = "Health")
	int OverHealthDecayAmount = 2; // Amount of health to decay every OverHealthDecayTime seconds

	UFUNCTION()
	void DecayOverHealth();

	FTimerHandle TimerHandle_OverHealthDecay;


	// ARMOUR -------------------------
	UPROPERTY()
	int Armour = 50;

	UPROPERTY(EditAnywhere, Category = "Armour")
	int StartingArmour = 50;

	UPROPERTY(EditAnywhere, Category = "Armour")
	float ArmourEfficiency = 0.67f; // Ratio of damage dealt that is applied to armour

	UPROPERTY(EditAnywhere, Category = "Armour")
	int BaseArmourMax = 75;

	UPROPERTY(EditAnywhere, Category = "Armour")
	int OverArmourMax = 175; // Max armour, including base-armour and overarmour

	UPROPERTY(EditAnywhere, Category = "Armour")
	float OverArmourDecayInterval = 1.f; // Time (seconds) between over-Armour decay ticks. 

	UPROPERTY(EditAnywhere, Category = "Armour")
	int OverArmourDecayAmount = 2; // Amount of Armour to decay every OverArmourDecayTime seconds

	UFUNCTION()
	void DecayOverArmour();

	FTimerHandle TimerHandle_OverArmourDecay;
		
};
