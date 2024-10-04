#include "AHealthComponent.h"


UAHealthComponent::UAHealthComponent()
{

}

bool UAHealthComponent::ApplyDamage(const int Amount, AActor* InstigatorActor)
{
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to apply negative [%d] damage"), Amount);
		return false;
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

	OnHealthChanged.Broadcast(this, InstigatorActor, Health, ActualHealthDelta, Armour, ActualArmourDelta);

	if (Health == 0)
	{
		// TODO Death logic via GameMode
	}

	return ActualHealthDelta != 0 || ActualArmourDelta != 0;
}

bool UAHealthComponent::AddHealth(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor)
{
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add negative [%d] health"), Amount);
		return false;
	}

	if (!bCanOverHeal && Health >= BaseHealthMax)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't add health as player already at or above BaseHealthMax"));
		return false;
	}

	const int OldHealth = Health;
	const int NewMaxHealth = bCanOverHeal ? OverHealthMax : BaseHealthMax;

	Health = FMath::Min((Health + Amount), NewMaxHealth);

	const int ActualHealthDelta = Health - OldHealth;

	OnHealthChanged.Broadcast(this, InstigatorActor, Health, ActualHealthDelta, Armour, 0);

    return true;
}

bool UAHealthComponent::AddArmour(const int Amount, const bool bCanOverHeal, AActor* InstigatorActor)
{
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to add negative [%d] armour"), Amount);
		return false;
	}

	if (!bCanOverHeal && Armour >= BaseArmourMax)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't add armour as player already at or above BaseArmourMax"));
		return false;
	}

	const int OldArmour = Armour;
	const int NewMaxArmour = bCanOverHeal ? OverHealthMax : BaseHealthMax;

	Health = FMath::Min((Armour + Amount), NewMaxArmour);

	const int ActualArmourDelta = Armour - OldArmour;

	OnHealthChanged.Broadcast(this, InstigatorActor, Health, 0, Armour, ActualArmourDelta);

	return true;
}





