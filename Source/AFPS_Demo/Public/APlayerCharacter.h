#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputActionValue.h"

#include "APlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;

class UAWeaponContainerComponent;
class UAStackComponent;

UCLASS()
class AFPS_DEMO_API AAPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAPlayerCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

protected:
	virtual void BeginPlay() override;

	// COMPONENTS -------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAWeaponContainerComponent> WeaponComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAStackComponent> HealthComp;

	// INPUT ------------------------------------------------------------
	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	// Move
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MovementAction;

	void Move(const FInputActionValue& Value);

	// Look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	void Look(const FInputActionValue& Value);

	// Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

};
