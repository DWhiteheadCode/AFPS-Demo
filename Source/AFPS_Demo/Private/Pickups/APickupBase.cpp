#include "Pickups/APickupBase.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AAPickupBase::AAPickupBase()
{
	CollisionSphereComp = CreateDefaultSubobject<USphereComponent>("CollisionSphereComp");
	CollisionSphereComp->SetSphereRadius(CollisionRadius);
	CollisionSphereComp->SetCollisionProfileName("Pickup");

	RootComponent = CollisionSphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	bReplicates = true;
}

void AAPickupBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CollisionSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAPickupBase::OnBeginOverlap);
}

void AAPickupBase::StartCooldown()
{
	if (!bRespawns)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start cooldown for pickup [%s], but bRespawns is false."), *GetNameSafe(this));
		return;
	}

	if (bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start pickup [%s] cooldown, but it was already on cooldown."), *GetNameSafe(this));
		return;
	}

	bIsOnCooldown = true;
	UpdatePickupState();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &AAPickupBase::OnCooldownEnd, CooldownDuration);
}

void AAPickupBase::OnCooldownEnd()
{
	if (!bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to end pickup [%s] cooldown, but it wasn't on cooldown."), *GetNameSafe(this));
		return;
	}

	bIsOnCooldown = false;
	UpdatePickupState();
}

void AAPickupBase::UpdatePickupState()
{
	SetActorEnableCollision(!bIsOnCooldown);
	MeshComp->SetVisibility(!bIsOnCooldown, true);
}

void AAPickupBase::OnRep_CooldownStateChanged()
{
	UpdatePickupState();
}

void AAPickupBase::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Functionality in base class
}

void AAPickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAPickupBase, bIsOnCooldown);
}