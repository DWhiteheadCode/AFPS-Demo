#include "APlayerCharacter.h"

#include "Camera/CameraComponent.h"

AAPlayerCharacter::AAPlayerCharacter()
{
	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->bUsePawnControlRotation = true;
}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

