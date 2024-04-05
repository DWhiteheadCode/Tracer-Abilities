// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction.h"

#include "TActionComponent.h"

void UTAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Started Action: %s"), *GetNameSafe(this));

	UTActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGameplayTags.AppendTags(GrantsTags);
	bIsRunning = true;
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
	if (IsRunning())
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

UTActionComponent* UTAction::GetOwningComponent() const
{
	return Cast<UTActionComponent>(GetOuter());
}
