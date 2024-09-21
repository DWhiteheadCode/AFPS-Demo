#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "APlayerCharacter.generated.h"

UCLASS()
class AFPS_DEMO_API AAPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
