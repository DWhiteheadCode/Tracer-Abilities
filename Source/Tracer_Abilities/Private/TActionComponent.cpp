// Fill out your copyright notice in the Description page of Project Settings.


#include "TActionComponent.h"

#include "TAction.h"
#include "GameplayTagContainer.h"

void UTActionComponent::AddAction(const TSubclassOf<UTAction> ActionClass)
{
	if (!ActionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't AddAction to ActionComponent: ActionClass was null."));
		return;
	}

	UTAction* const NewAction = NewObject<UTAction>(this, ActionClass);
	if (NewAction)
	{
		NewAction->BeginPlay();
		Actions.Add(NewAction);
		OnActionAdded.Broadcast(NewAction);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't AddAction to ActionComponent: NewAction creation failed."));
	}
}

bool UTActionComponent::StartActionByTag(const FGameplayTag ActionTag)
{
	for (UTAction* const Action : Actions)
	{
		if (Action && Action->IdentifierTag == ActionTag)
		{
			if (! Action->CanStart())
			{
				const FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionTag.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);

				continue;
			}

			Action->StartAction();
			return true;
		}
	}

	return false;
}

bool UTActionComponent::StopActionByTag(const FGameplayTag ActionTag)
{
	for (UTAction* const Action : Actions)
	{
		if (Action && Action->IdentifierTag == ActionTag)
		{
			if (Action->IsRunning())
			{
				Action->StopAction();
				return true;
			}			
		}
	}

	return false;
}

void UTActionComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TSubclassOf<UTAction> ActionClass : DefaultActions)
	{
		AddAction(ActionClass);
	}
}
