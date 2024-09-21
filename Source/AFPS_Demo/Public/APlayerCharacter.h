#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "APlayerCharacter.generated.h"

class UCameraComponent;

UCLASS()
class AFPS_DEMO_API AAPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAPlayerCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComp;

};
