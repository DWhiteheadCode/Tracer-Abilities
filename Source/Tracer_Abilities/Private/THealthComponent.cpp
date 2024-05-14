// Fill out your copyright notice in the Description page of Project Settings.


#include "THealthComponent.h"

UTHealthComponent::UTHealthComponent()
{
}

int UTHealthComponent::GetHealth() const
{
	return Health;
}

int UTHealthComponent::GetHealthMax() const
{
	return HealthMax;
}

void UTHealthComponent::ApplyDamage(int Damage)
{
	if (!ensure(Damage >= 0))
	{
		return;
	}

	int OldHealth = Health;

	Health = FMath::Max(0, Health - Damage);

	int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}	
}

void UTHealthComponent::ApplyHeal(int Amount)
{
	if (!ensure(Amount >= 0))
	{
		return;
	}

	int OldHealth = Health;

	Health = FMath::Min(Health + Amount, HealthMax);

	int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}
}

void UTHealthComponent::SetHealth(int NewHealth)
{
	int OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0, HealthMax);

	int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}
}

