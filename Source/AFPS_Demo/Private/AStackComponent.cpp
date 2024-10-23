#include "AStackComponent.h"

#include "Net/UnrealNetwork.h"

UAStackComponent::UAStackComponent()
{
	SetIsReplicatedByDefault(true);
}

void UAStackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (BaseHealthMax > OverHealthMax)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseHealthMax must be <= OverHealthMax (OverHealthMax is the total max health, including BaseHealth and OverHealth)."));
		return;
	}

	if (BaseArmourMax > OverArmourMax)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseArmourMax must be <= OverArmourMax (OverArmourMax is the total max armour, including BaseArmour and OverArmour)."));
		return;
	}	

	if (GetOwner()->HasAuthority())
	{
		Health = StartingHealth;
		Armour = StartingArmour;

		MulticastStackChanged(this, GetOwner(), Health, Health, Armour, Armour, (Health + Armour));

		// Started with over-health, so start decay timer
		if (Health > BaseHealthMax && CanDecayOverHealth())
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverHealthDecay, this, &UAStackComponent::DecayOverHealth, OverHealthDecayIntervalSeconds);
		}

		// Started with over-armour, so start decay timer
		if (Armour > BaseArmourMax && CanDecayOverArmour())
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverArmourDecay, this, &UAStackComponent::DecayOverArmour, OverArmourDecayIntervalSeconds);
		}
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
		UE_LOG(LogTemp, Error, TEXT("Tried to apply damage, but BaseHealthMax [%d] exceeded OverHealthMax [%d]."), BaseHealthMax, OverHealthMax);
		return false;
	}

	if (BaseArmourMax > OverArmourMax)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to apply damage, but BaseArmourMax [%d] exceeded OverArmourMax [%d]."), BaseArmourMax, OverArmourMax);
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

	const int NewHealth = FMath::Max((OldHealth - HealthPortion), 0);
	const int NewArmour = FMath::Max((OldArmour - ArmourPortion), 0);

	const int ActualHealthDelta = NewHealth - OldHealth;
	const int ActualArmourDelta = NewArmour - OldArmour;

	// Actually apply changes if on server.
	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;
		Armour = NewArmour;
		MulticastStackChanged(this, InstigatorActor, NewHealth, ActualHealthDelta, NewArmour, ActualArmourDelta, (ActualHealthDelta + ActualArmourDelta));

		if (Health == 0)
		{
			// TODO Death logic via GameMode
		}
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
		//UE_LOG(LogTemp, Log, TEXT("Skipping AddHealth with Amount == 0"));
		return false;
	}

	if (!bCanOverHeal && Health >= BaseHealthMax)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't add health as player already at or above BaseHealthMax, and this heal can't overheal"));
		return false;
	}

	const int OldHealth = Health;
	const int NewMaxHealth = bCanOverHeal ? OverHealthMax : BaseHealthMax;
	const int NewHealth = FMath::Min((Health + Amount), NewMaxHealth);
	const int ActualHealthDelta = NewHealth - OldHealth;

	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;
		MulticastStackChanged(this, InstigatorActor, Health, ActualHealthDelta, Armour, 0, ActualHealthDelta);

		// This healed overhealth, so set/reset decay timer
		if (bCanOverHeal && Health > BaseHealthMax && CanDecayOverHealth())
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverHealthDecay, this, &UAStackComponent::DecayOverHealth, OverHealthDecayIntervalSeconds);
		}
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
		//UE_LOG(LogTemp, Log, TEXT("Skipping AddArmour with Amount == 0"));
		return false;
	}

	if (!bCanOverHeal && Armour >= BaseArmourMax)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't add armour as player already at or above BaseArmourMax, and this heal can't overheal"));
		return false;
	}

	const int OldArmour = Armour;
	const int NewMaxArmour = bCanOverHeal ? OverArmourMax : BaseArmourMax;
	const int NewArmour = FMath::Min((Armour + Amount), NewMaxArmour);
	const int ActualArmourDelta = NewArmour - OldArmour;

	if (GetOwner()->HasAuthority())
	{
		Armour = NewArmour;
		MulticastStackChanged(this, InstigatorActor, Health, 0, Armour, ActualArmourDelta, ActualArmourDelta);

		// This healed overarmour, so set/reset decay timer
		if (bCanOverHeal && Armour > BaseArmourMax && CanDecayOverArmour())
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverArmourDecay, this, &UAStackComponent::DecayOverArmour, OverArmourDecayIntervalSeconds);
		}
	}
	
	return true;
}



void UAStackComponent::DecayOverHealth()
{
	if (!CanDecayOverHealth())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to decay Health, but CanDecayOverHealth() was false."));
		return;
	}

	if (Health <= BaseHealthMax)
	{
		UE_LOG(LogTemp, Log, TEXT("No need to decay OverHealth as user has no OverHealth"));
		return;
	}

	const int AmountToDecay = FMath::Min( Health - BaseHealthMax, OverHealthDecayAmount );
	const int NewHealth = Health - AmountToDecay;

	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;
		MulticastStackChanged(this, GetOwner(), Health, AmountToDecay, Armour, 0, AmountToDecay);

		// More health needs to be decayed
		if (Health > BaseHealthMax)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverHealthDecay, this, &UAStackComponent::DecayOverHealth, OverHealthDecayIntervalSeconds);
		}
	}
}

void UAStackComponent::DecayOverArmour()
{
	if (!CanDecayOverArmour())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to decay Armour, but CanDecayOverArmour() was false."));
		return;
	}

	if (Armour <= BaseArmourMax)
	{
		UE_LOG(LogTemp, Log, TEXT("No need to decay OverArmour as user has no OverArmour"));
		return;
	}

	const int AmountToDecay = FMath::Min(Armour - BaseArmourMax, OverArmourDecayAmount);
	const int NewArmour = Armour - AmountToDecay;

	if (GetOwner()->HasAuthority())
	{
		Armour = NewArmour;
		MulticastStackChanged(this, GetOwner(), Health, 0, Armour, AmountToDecay, AmountToDecay);

		// More armour needs to be decayed
		if (Armour > BaseArmourMax)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_OverArmourDecay, this, &UAStackComponent::DecayOverArmour, OverArmourDecayIntervalSeconds);
		}
	}	
}

int UAStackComponent::GetHealth() const
{
	return Health;
}

int UAStackComponent::GetArmour() const
{
	return Armour;
}

bool UAStackComponent::CanDecayOverHealth() const
{
	return OverHealthDecayAmount > 0 && OverHealthDecayIntervalSeconds > 0;
}

bool UAStackComponent::CanDecayOverArmour() const
{
	return OverArmourDecayAmount > 0 && OverArmourDecayIntervalSeconds > 0;
}

void UAStackComponent::MulticastStackChanged_Implementation(UAStackComponent* OwningComp, AActor* InstigatorActor, 
	int NewHealth, int DeltaHealth, int NewArmour, int DeltaArmour, int TotalDelta)
{
	OnStackChanged.Broadcast(OwningComp, InstigatorActor, NewHealth, DeltaHealth, NewArmour, DeltaArmour, TotalDelta);
}

void UAStackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Of these, only "Health" and "Armour" are expected to change during normal execution, and 
	// several should only be used by the server.
	// TODO - Check if it is better to replicate the other properties to ensure consistency, or 
	//        not replicate them to save bandwidth etc.

	DOREPLIFETIME(UAStackComponent, Health);
	DOREPLIFETIME(UAStackComponent, StartingHealth);
	DOREPLIFETIME(UAStackComponent, BaseHealthMax);
	DOREPLIFETIME(UAStackComponent, OverHealthMax);
	DOREPLIFETIME(UAStackComponent, OverHealthDecayIntervalSeconds);
	DOREPLIFETIME(UAStackComponent, OverHealthDecayAmount);

	DOREPLIFETIME(UAStackComponent, Armour);
	DOREPLIFETIME(UAStackComponent, StartingArmour);
	DOREPLIFETIME(UAStackComponent, BaseArmourMax);
	DOREPLIFETIME(UAStackComponent, OverArmourMax);
	DOREPLIFETIME(UAStackComponent, OverArmourDecayIntervalSeconds);
	DOREPLIFETIME(UAStackComponent, OverArmourDecayAmount);
}