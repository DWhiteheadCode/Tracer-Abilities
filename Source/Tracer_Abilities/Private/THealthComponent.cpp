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
	if (Damage < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to apply negative damage (%i) to HealthComponent. Health will not change."), Damage);
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
	if (Amount < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to apply negative heal (%i) to HealthComponent. Health will not change."), Amount);
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
	if (NewHealth < 0 || NewHealth > HealthMax)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to set HealthComponent Health to an out-of-bounds value [%i] which is < 0 or > HealthMax [%i]. Health will be updated, but clamped to this range."), NewHealth, HealthMax);
	}

	const int OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0, HealthMax);

	const int ActualDelta = Health - OldHealth;

	if (ActualDelta != 0)
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}
}

