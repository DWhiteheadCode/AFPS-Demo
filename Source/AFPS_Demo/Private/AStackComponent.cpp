#include "AStackComponent.h"


UAStackComponent::UAStackComponent()
{

}

void UAStackComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = StartingHealth;
	Armour = StartingArmour;

	if (BaseHealthMax > OverHealthMax)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseHealthMax must be <= OverHealthMax."));
		return;
	}

	if (BaseArmourMax > OverArmourMax)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseArmourMax must be <= OverArmourMax."));
		return;
	}

	// Started with overhealth, so start decay timer
	if (Health > BaseHealthMax)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverHealthDecay, this, &UAStackComponent::DecayOverHealth, OverHealthDecayInterval);
	}

	// Started with overarmour, so start decay timer
	if (Armour > BaseArmourMax)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverArmourDecay, this, &UAStackComponent::DecayOverArmour, OverArmourDecayInterval);
	}
}

bool UAStackComponent::ApplyDamage(int Amount, AActor* InstigatorActor)
{
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to apply negative [%d] damage"), Amount);
		return false;
	}

	if (BaseHealthMax > OverHealthMax)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseHealthMax [%d] can't exceed OverHealthMax [%d]."), BaseHealthMax, OverHealthMax);
		return false;
	}

	if (BaseArmourMax > OverArmourMax)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseArmourMax [%d] can't exceed OverArmourMax [%d]."), BaseArmourMax, OverArmourMax);
		return false;
	}


	if (InstigatorActor == GetOwner())
	{
		Amount *= SelfDamageMultiplier;
	}


	// Do as much damage to Armour as possible, then apply remaining damage to Health
	const int ArmourPortion = FMath::Min(Armour, Amount * ArmourEfficiency); 
	const int HealthPortion = Amount - ArmourPortion;

	const int OldHealth = Health;
	const int OldArmour = Armour;

	Health = FMath::Max( (Health - HealthPortion), 0 );
	Armour = FMath::Max( (Armour - ArmourPortion), 0 );

	const int ActualHealthDelta = Health - OldHealth;
	const int ActualArmourDelta = Armour - OldArmour;

	OnStackChanged.Broadcast(this, InstigatorActor, Health, ActualHealthDelta, Armour, ActualArmourDelta);

	if (Health == 0)
	{
		// TODO Death logic via GameMode
	}

	return true;
}

bool UAStackComponent::AddHealth(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor)
{
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add negative [%d] health"), Amount);
		return false;
	}

	if (Amount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Skipping AddHealth with Amount == 0"));
		return false;
	}

	if (!bCanOverHeal && Health >= BaseHealthMax)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't add health as player already at or above BaseHealthMax, and this heal can't overheal"));
		return false;
	}

	const int OldHealth = Health;
	const int NewMaxHealth = bCanOverHeal ? OverHealthMax : BaseHealthMax;

	Health = FMath::Min((Health + Amount), NewMaxHealth);

	const int ActualHealthDelta = Health - OldHealth;

	OnStackChanged.Broadcast(this, InstigatorActor, Health, ActualHealthDelta, Armour, 0);

	// This healed overhealth, so set/reset decay timer
	if (bCanOverHeal && Health > BaseHealthMax)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverHealthDecay, this, &UAStackComponent::DecayOverHealth, OverHealthDecayInterval);
	}

    return true;
}

bool UAStackComponent::AddArmour(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor)
{
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add negative [%d] armour"), Amount);
		return false;
	}

	if (Amount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Skipping AddArmour with Amount == 0"));
		return false;
	}

	if (!bCanOverHeal && Armour >= BaseArmourMax)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't add armour as player already at or above BaseArmourMax, and this heal can't overheal"));
		return false;
	}

	const int OldArmour = Armour;
	const int NewMaxArmour = bCanOverHeal ? OverArmourMax : BaseArmourMax;

	Armour = FMath::Min((Armour + Amount), NewMaxArmour);

	const int ActualArmourDelta = Armour - OldArmour;

	OnStackChanged.Broadcast(this, InstigatorActor, Health, 0, Armour, ActualArmourDelta);

	// This healed overarmour, so set/reset decay timer
	if (bCanOverHeal && Armour > BaseArmourMax)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverArmourDecay, this, &UAStackComponent::DecayOverArmour, OverArmourDecayInterval);
	}

	return true;
}

int UAStackComponent::GetHealth() const
{
	return Health;
}

int UAStackComponent::GetArmour() const
{
	return Armour;
}

void UAStackComponent::DecayOverHealth()
{
	if (OverHealthDecayAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverHealthDecayAmount is non-positive [%d]. Ignoring decay."), OverHealthDecayAmount);
		return;
	}

	if (Health <= BaseHealthMax)
	{
		UE_LOG(LogTemp, Log, TEXT("No need to decay OverHealth as user has no OverHealth"));
		return;
	}

	const int AmountToDecay = FMath::Min( Health - BaseHealthMax, OverHealthDecayAmount );
	const int OldHealth = Health;

	Health -= AmountToDecay;
	
	OnStackChanged.Broadcast(this, GetOwner(), Health, Health - OldHealth, Armour, 0);

	if (OverHealthDecayInterval <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverHealthDecayInterval is non-positive [%f]. Not setting decay timer."), OverHealthDecayInterval);
		return;
	}

	// More health needs to be decayed
	if (Health > BaseHealthMax)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverHealthDecay, this, &UAStackComponent::DecayOverHealth, OverHealthDecayInterval);
	}
}

void UAStackComponent::DecayOverArmour()
{
	if (OverArmourDecayAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverArmourDecayAmount is non-positive [%d]. Ignoring decay."), OverArmourDecayAmount);
		return;
	}

	if (Armour <= BaseArmourMax)
	{
		UE_LOG(LogTemp, Log, TEXT("No need to decay OverArmour as user has no OverArmour"));
		return;
	}

	const int AmountToDecay = FMath::Min(Armour - BaseArmourMax, OverArmourDecayAmount);
	const int OldArmour = Armour;

	Armour -= AmountToDecay;

	OnStackChanged.Broadcast(this, GetOwner(), Health, 0, Armour, Armour - OldArmour);

	if (OverArmourDecayInterval <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverArmourDecayInterval is non-positive [%f]. Not setting decay timer."), OverArmourDecayInterval);
		return;
	}

	// More armour needs to be decayed
	if (Armour > BaseArmourMax)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverArmourDecay, this, &UAStackComponent::DecayOverArmour, OverArmourDecayInterval);
	}
}