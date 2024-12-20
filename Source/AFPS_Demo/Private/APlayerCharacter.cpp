#include "APlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "Weapons/AWeaponContainerComponent.h"
#include "AStackComponent.h"

AAPlayerCharacter::AAPlayerCharacter()
{
	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->bUsePawnControlRotation = true;
	CameraComp->SetRelativeLocation(FVector(0, 0, 90)); // Place camera in mesh's head

	WeaponComp = CreateDefaultSubobject<UAWeaponContainerComponent>("WeaponComp");
	StackComp = CreateDefaultSubobject<UAStackComponent>("StackComp");

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh)
	{
		CharacterMesh->bOwnerNoSee = true;
	}
}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* const PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem
			= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}
}

void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* const EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AAPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAPlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AAPlayerCharacter::Jump);
	}
}

void AAPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (GetController())
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		AddMovementInput(GetActorForwardVector(), MoveValue.X);
		AddMovementInput(GetActorRightVector(), MoveValue.Y);
	}
}

void AAPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (GetController())
	{
		const FVector2D LookValue = Value.Get<FVector2D>();
		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}

FVector AAPlayerCharacter::GetPawnViewLocation() const
{
	return CameraComp->GetComponentLocation();
}