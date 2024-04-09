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

void UTAction::StartAction_Implementation()
{
	if (!ensureAlways(CanStart()))
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
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ActiveDuration, this, &UTAction::StopAction, ActiveDuration, false);
}

void UTAction::StopAction_Implementation()
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

bool UTAction::CanStart_Implementation()
{
	if (IsRunning())
	{
		UE_LOG(LogTemp, Log, TEXT("Action [%s] can't start: Already running"), *GetNameSafe(this));
		return false;
	}

	if (bIsOnCooldown)
	{
		UE_LOG(LogTemp, Log, TEXT("Action [%s] can't start: On Cooldown"), *GetNameSafe(this));
		return false;
	}

	UTActionComponent* ActionComp = GetOwningComponent();

	if (ActionComp->ActiveGameplayTags.HasAny(BlockedByTags))
	{
		UE_LOG(LogTemp, Log, TEXT("Action [%s] can't start: Blocked By Tags"), *GetNameSafe(this));
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
	UE_LOG(LogTemp, Log, TEXT("Action cooldown ended: %s"), *GetNameSafe(this));
	bIsOnCooldown = false;
}

UTActionComponent* UTAction::GetOwningComponent() const
{
	return Cast<UTActionComponent>(GetOuter());
}