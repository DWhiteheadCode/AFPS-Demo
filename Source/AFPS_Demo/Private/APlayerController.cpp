#include "APlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#include "AStackComponent.h"

void AAPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AAPlayerController::SetPawn(APawn* NewPawn)
{
	Super::SetPawn(NewPawn);

	if (!IsLocalController())
	{
		return;
	}

	if (NewPawn)
	{
		UAStackComponent* StackComp = Cast<UAStackComponent>(NewPawn->GetComponentByClass(UAStackComponent::StaticClass()));

		if (StackComp)
		{
			StackComp->OnStackChanged.AddUniqueDynamic(this, &AAPlayerController::OnPawnStackChanged);
		}
	}
}

void AAPlayerController::OnPawnStackChanged(UAStackComponent* OwningComp, AActor* InstigatorActor, int NewHealth, int DeltaHealth, int NewArmour, int DeltaArmour, int TotalDelta)
{
	if (TotalDelta < 0.f)
	{
		OnDamageTaken.Broadcast(this, OwningComp, InstigatorActor, -TotalDelta);

		if (IsLocalController())
		{
			PlayIncomingDamageSound(InstigatorActor);
		}
	}
}

void AAPlayerController::PlayIncomingDamageSound(AActor* InstigatorActor)
{
	if (!InstigatorActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to play incoming damage sound, but InstigatorActor was null"));
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to play incoming damage sound, but ControlledPawn was null"));
		return;
	}

	if (!IncomingDamageSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to play incoming damage sound, but IncomingDamageSound wasn't set in PlayerController [%s] blueprint"), *GetNameSafe(this));
		return;
	}

	const FVector ViewLocation = ControlledPawn->GetPawnViewLocation();
	
	FVector DamageSoundLocation = InstigatorActor->GetActorLocation();
	DamageSoundLocation.Z = ViewLocation.Z; // Ignore vertical differences

	// Set the damage sound in the direction of the instigator, but at a fixed distance
	FVector DamageSoundDirection =  DamageSoundLocation - ViewLocation;
	DamageSoundDirection.Normalize();
	DamageSoundLocation = ViewLocation + (IncomingDamageSoundDistance * DamageSoundDirection);

	//DrawDebugSphere(GetWorld(), DamageSoundLocation, 5.f, 16, FColor::White, false, 2.f, 0, 1.f);

	UGameplayStatics::PlaySoundAtLocation(this, IncomingDamageSound, DamageSoundLocation);
}

void AAPlayerController::ClientOnDamageDealt_Implementation(UAStackComponent* TargetStackComp, const int DamageAmount)
{
	OnDamageDealt.Broadcast(this, TargetStackComp, DamageAmount);
}
