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

void AAPickupBase::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CanPickup(OtherActor))
	{
		Pickup(OtherActor);
	}	
}

void AAPickupBase::Pickup(AActor* OtherActor)
{
	bIsActive = false;
	OnRep_IsActive();

	if (bRespawns)
	{
		StartCooldown();
	}
	else
	{
		SetLifeSpan(2.f);
	}
}

// Derived classes should typically override this.
bool AAPickupBase::CanPickup(AActor* OtherActor)
{
	return OtherActor && HasAuthority();
}

void AAPickupBase::StartCooldown()
{
	if (!bRespawns)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start cooldown for pickup [%s], but bRespawns is false."), *GetNameSafe(this));
		return;
	}

	if (bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start pickup [%s] cooldown, but it was active."), *GetNameSafe(this));
		return;
	}

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client tried to start pickup [%s] coolodown."), *GetNameSafe(this));
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &AAPickupBase::OnCooldownEnd, CooldownDuration);
}

void AAPickupBase::OnCooldownEnd()
{
	if (bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to end pickup [%s] cooldown, but it was already active."), *GetNameSafe(this));
		return;
	}

	bIsActive = true;
	OnRep_IsActive();
}

void AAPickupBase::OnRep_IsActive()
{
	SetActorEnableCollision(bIsActive);
	MeshComp->SetVisibility(bIsActive, true);
}

void AAPickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAPickupBase, bIsActive);
}