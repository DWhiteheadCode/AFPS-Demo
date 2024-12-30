#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "APlayerController.generated.h"

class USoundCue;
class UInputMappingContext;
class UInputAction;

class UAStackComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnDamageTaken,
	AAPlayerController*, OwningPlayer,
	UAStackComponent*, StackComp,
	AActor*, InstigatorActor,
	int, DamageAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnDamageDealt,
	AAPlayerController*, OwningPlayer,
	UAStackComponent*, TargetStackComp,
	int, DamageAmount
);

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API AAPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void BeginPlay() override;

	// DAMAGE RECEIVED ----------------------------------------------------------------------
	void SetPawn(APawn* NewPawn) override;

	UFUNCTION()
	void OnPawnStackChanged(UAStackComponent* OwningComp, AActor* InstigatorActor,
		int NewHealth, int DeltaHealth, int NewArmour, int DeltaArmour, int TotalDelta);

	UPROPERTY(BlueprintAssignable)
	FOnDamageTaken OnDamageTaken;

	// DAMAGE DEALT --------------------------------------------------------------------------
	UFUNCTION(Client, Unreliable)
	void ClientOnDamageDealt(UAStackComponent* TargetStackComp, const int DamageAmount);

	UPROPERTY(BlueprintAssignable)
	FOnDamageDealt OnDamageDealt;

protected:
	void SetupInputComponent() override;

	// PAUSE MENU -----------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY() 
	UUserWidget* PauseMenuInstance;

	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* MenusMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PauseMenuAction;

	// INCOMING DAMAGE ------------------------------------------------------------------------
	UFUNCTION()
	void PlayIncomingDamageSound(AAPlayerController* OwningPlayer, UAStackComponent* StackComp, AActor* InstigatorActor, int DamageAmount);

	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundCue> IncomingDamageSound;

	UPROPERTY(EditDefaultsOnly, Category="Sound")
	float IncomingDamageSoundDistance = 250.f;

};
