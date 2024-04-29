// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction.h"

#include "TActionComponent.h"

UTAction::UTAction()
{
	bIsRunning = false;
	Cooldown = 15;
	ActiveDuration = 5;
	bSetAutoEndTimer = true;
	MaxCharges = 1;
	CurrentCharges = 1;
}

void UTAction::BeginPlay()
{
	// Overridden by derived classes
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
	
	CurrentCharges--;

	// Don't reset a cooldown if it is already active. 
	if ( ! GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Cooldown))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &UTAction::OnCooldownEnd, Cooldown, false);
	}	
	
	// Start active duration
	if (bSetAutoEndTimer)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ActiveDuration, this, &UTAction::StopAction, ActiveDuration, false);
	}	
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

	if (CurrentCharges <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Action [%s] can't start: No Charges"), *GetNameSafe(this));
		return false;
	}

	if (ActiveDuration <= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("Action [%s] can't start: Invalid ActiveDuration (must be >= 0)"), *GetNameSafe(this));
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

int UTAction::GetMaxCharges() const
{
	return MaxCharges;
}

int UTAction::GetCurrentCharges() const
{
	return CurrentCharges;
}

void UTAction::OnCooldownEnd()
{
	UE_LOG(LogTemp, Log, TEXT("Action cooldown ended: %s"), *GetNameSafe(this));
	CurrentCharges = FMath::Min( (CurrentCharges + 1), MaxCharges );

	// Start another cooldown if more charges could be stored
	if (CurrentCharges < MaxCharges)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &UTAction::OnCooldownEnd, Cooldown, false);
	}
}

UTActionComponent* UTAction::GetOwningComponent() const
{
	return Cast<UTActionComponent>(GetOuter());
}