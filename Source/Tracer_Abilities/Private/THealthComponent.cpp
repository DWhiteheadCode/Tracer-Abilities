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

void UTHealthComponent::ApplyDamage(const int Damage)
{
	if (!ensure(Damage >= 0))
	{
		return;
	}

	const int OldHealth = Health;

	Health = FMath::Max(0, Health - Damage);

	const int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}	
}

void UTHealthComponent::ApplyHeal(const int Amount)
{
	if (!ensure(Amount >= 0))
	{
		return;
	}

	const int OldHealth = Health;

	Health = FMath::Min(Health + Amount, HealthMax);

	const int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}
}

void UTHealthComponent::SetHealth(const int NewHealth)
{
	const int OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0, HealthMax);

	const int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}
}

