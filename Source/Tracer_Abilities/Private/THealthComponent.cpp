// Fill out your copyright notice in the Description page of Project Settings.


#include "THealthComponent.h"

UTHealthComponent::UTHealthComponent()
{
	Health = 100;
	HealthMax = 100;
}

float UTHealthComponent::GetHealth() const
{
	return Health;
}

float UTHealthComponent::GetHealthMax() const
{
	return HealthMax;
}

void UTHealthComponent::ApplyDamage(float Damage)
{
	if (!ensure(Damage >= 0))
	{
		return;
	}

	float OldHealth = Health;

	Health = FMath::Max(0, Health - Damage);

	float ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}	
}

