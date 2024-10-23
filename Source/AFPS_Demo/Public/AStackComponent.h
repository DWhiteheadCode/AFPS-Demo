#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AStackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(
	FOnStackChanged,
	UAStackComponent*, OwningComp,
	AActor*, InstigatorActor,
	int, NewHealth,
	int, DeltaHealth,
	int, NewArmour,
	int, DeltaArmour,
	int, TotalDelta
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AFPS_DEMO_API UAStackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAStackComponent();

	void BeginPlay() override;
	
	// STACK MUTATORS ----------------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	bool ApplyDamage(int Amount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable)
	bool AddHealth(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable)
	bool AddArmour(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStackChanged(UAStackComponent* OwningComp, AActor* InstigatorActor, int NewHealth, int DeltaHealth, int NewArmour, int DeltaArmour, int TotalDelta);

	UPROPERTY(BlueprintAssignable)
	FOnStackChanged OnStackChanged;

	// STACK ACCESSORS ---------------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	int GetHealth() const;

	UFUNCTION(BlueprintCallable)
	int GetArmour() const;


protected:
	// GENERAL ------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Stack")
	float SelfDamageMultiplier = 0.67f;

	// HEALTH -------------------------------------------------------------------------------
	UPROPERTY(Replicated)
	int Health = 125;

	UPROPERTY(EditAnywhere, Replicated, Category = "Health")
	int StartingHealth = 125;

	UPROPERTY(EditAnywhere, Replicated, Category = "Health")
	int BaseHealthMax = 100;

	UPROPERTY(EditAnywhere, Replicated, Category = "Health")
	int OverHealthMax = 175; // Max health, including base-health and overhealth

	UPROPERTY(EditAnywhere, Replicated, Category = "Health")
	float OverHealthDecayIntervalSeconds = 1.f; // Time between over-health decay ticks. 

	UPROPERTY(EditAnywhere, Replicated, Category = "Health")
	int OverHealthDecayAmount = 2; // Amount of OverHealth to decay every OverHealthDecayIntervalSeconds

	UFUNCTION()
	bool CanDecayOverHealth() const;

	UFUNCTION()
	void DecayOverHealth();

	FTimerHandle TimerHandle_OverHealthDecay;


	// ARMOUR -------------------------
	UPROPERTY(Replicated)
	int Armour = 50;

	UPROPERTY(EditAnywhere, Replicated, Category = "Armour")
	float ArmourEfficiency = 0.67f; // Ratio of damage dealt that is applied to armour

	UPROPERTY(EditAnywhere, Replicated, Category = "Armour")
	int StartingArmour = 50;	

	UPROPERTY(EditAnywhere, Replicated, Category = "Armour")
	int BaseArmourMax = 75;

	UPROPERTY(EditAnywhere, Replicated, Category = "Armour")
	int OverArmourMax = 175; // Max armour, including base-armour and overarmour

	UPROPERTY(EditAnywhere, Replicated, Category = "Armour")
	float OverArmourDecayIntervalSeconds = 1.f; // Time between over-Armour decay ticks. 

	UPROPERTY(EditAnywhere, Replicated, Category = "Armour")
	int OverArmourDecayAmount = 2; // Amount of OverArmour to decay every OverArmourDecayIntervalSeconds

	UFUNCTION()
	bool CanDecayOverArmour() const;

	UFUNCTION()
	void DecayOverArmour();

	FTimerHandle TimerHandle_OverArmourDecay;
};
