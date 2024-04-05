// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction.h"

#include "TActionComponent.h"

UTAction::UTAction()
{
	bIsRunning = false;
	bIsOnCooldown = false;
	Cooldown = 15;
	ActiveDuration = 5;
}

void UTAction::StartAction_Implementation(AActor* Instigator)
{
	if (!ensureAlways(CanStart(Instigator)))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Started Action: %s"), *GetNameSafe(this));

	UTActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGameplayTags.AppendTags(GrantsTags);
	bIsRunning = true;
	
	// Start cooldown
	bIsOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &UTAction::OnCooldownEnd, Cooldown, false);
	
	// Start active duration
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "StopAction", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ActiveDuration, Delegate, ActiveDuration, false);
}

void UTAction::StopAction_Implementation(AActor* Instigator)
{
	if ( ! ensureAlways(bIsRunning))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Stopped Action: %s"), *GetNameSafe(this));

	UTActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGameplayTags.RemoveTags(GrantsTags);

	bIsRunning = false;
}

bool UTAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning() || bIsOnCooldown)
	{
		return false;
	}

	UTActionComponent* ActionComp = GetOwningComponent();

	if (ActionComp->ActiveGameplayTags.HasAny(BlockedByTags))
	{
		return false;
	}

	return true;
}

bool UTAction::IsRunning() const
{
	return bIsRunning;
}

void UTAction::OnCooldownEnd()
{
	bIsOnCooldown = false;
}

UTActionComponent* UTAction::GetOwningComponent() const
{
	return Cast<UTActionComponent>(GetOuter());
}