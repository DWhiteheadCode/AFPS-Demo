#include "ABasePickup.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AABasePickup::AABasePickup()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetSphereRadius(CollisionRadius);
	SphereComp->SetCollisionProfileName("Pickup");
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);
}

void AABasePickup::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AABasePickup::OnBeginOverlap);
}

void AABasePickup::StartCooldown()
{
	if (bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start pickup cooldown, but it was already on cooldown."));
		return;
	}

	bIsOnCooldown = true;
	UpdatePickupState();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &AABasePickup::OnCooldownEnd, CooldownDuration);
}

void AABasePickup::OnCooldownEnd()
{
	if (!bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to end pickup cooldown, but it wasn't on cooldown."));
		return;
	}

	bIsOnCooldown = false;
	UpdatePickupState();
}

void AABasePickup::UpdatePickupState()
{
	SetActorEnableCollision(!bIsOnCooldown);
	MeshComp->SetVisibility(!bIsOnCooldown, true);
}

void AABasePickup::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Functionality in base class
}