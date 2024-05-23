// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction.h"

#include "TActionComponent.h"

UTAction::UTAction()
{
}

void UTAction::BeginPlay()
{
	if (StartingCharges > MaxCharges)
	{
		UE_LOG(LogTemp, Warning, TEXT("Action [%s] has more StartingCharges than MaxCharges. Setting StargingCharges = MaxCharges."), *GetNameSafe(this));
		StartingCharges = MaxCharges;
	}

	CurrentCharges = StartingCharges;

	if (CurrentCharges < MaxCharges)
	{
		StartCooldown();
	}
}

void UTAction::StartAction_Implementation()
{
	if (!CanStart())
	{
		UE_LOG(LogTemp, Log, TEXT("Tried to start action [%s], but CanStart() returned false"), *GetNameSafe(this));
		return;
	}

	UTActionComponent* const ActionComp = GetOwningComponent();

	if (!ActionComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to start action [%s] as it didn't have an owning ActionComponent"), *GetNameSafe(this));
		return;
	}

	ActionComp->ActiveGameplayTags.AppendTags(GrantsTags);
	bIsRunning = true;	
	
	CurrentCharges--;

	// Don't reset a cooldown if one is already active. 
	if ( ! GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Cooldown))
	{
		StartCooldown();
	}	
	
	if (bSetAutoEndTimer)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ActiveDuration, this, &UTAction::StopAction, ActiveDuration, false);
	}	

	OnActionStarted.Broadcast(CurrentCharges);
	UE_LOG(LogTemp, Log, TEXT("Started Action: %s"), *GetNameSafe(this));
}

void UTAction::StopAction_Implementation()
{
	if (!bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to stop Action [%s], but it wasn't running"), *GetNameSafe(this));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Stopped Action: %s"), *GetNameSafe(this));

	UTActionComponent* const ActionComp = GetOwningComponent();

	if (!ActionComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to stop action [%s] as it didn't have an owning ActionComponent"), *GetNameSafe(this));
		return;
	}

	ActionComp->ActiveGameplayTags.RemoveTags(GrantsTags);

	bIsRunning = false;
}

bool UTAction::CanStart() const
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
		UE_LOG(LogTemp, Log, TEXT("Action [%s] can't start: Invalid ActiveDuration (must be > 0)"), *GetNameSafe(this));
		return false;
	}

	UTActionComponent* const ActionComp = GetOwningComponent();
	if (!ActionComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Action [%s] can't start: No ActionComponent"), *GetNameSafe(this));
		return false;
	}

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

FText UTAction::GetNameText_Implementation() const
{
	return FText::GetEmpty();
}

void UTAction::StartCooldown()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Cooldown, this, &UTAction::OnCooldownEnd, Cooldown, false);
	OnCooldownStarted.Broadcast(Cooldown);
}

void UTAction::OnCooldownEnd()
{
	UE_LOG(LogTemp, Log, TEXT("Action cooldown ended: %s"), *GetNameSafe(this));
	CurrentCharges = FMath::Min( (CurrentCharges + 1), MaxCharges );

	// Start another cooldown if more charges could be stored
	if (CurrentCharges < MaxCharges)
	{
		StartCooldown();
	}

	OnCooldownEnded.Broadcast(CurrentCharges);
}

float UTAction::GetRemainingCooldown() const
{
	FTimerManager& Manager = GetWorld()->GetTimerManager();

	if (Manager.IsTimerActive(TimerHandle_Cooldown))
	{
		return Manager.GetTimerRemaining(TimerHandle_Cooldown);
	}

	return 0.0f;
}

UTActionComponent* UTAction::GetOwningComponent() const
{
	return Cast<UTActionComponent>(GetOuter());
}