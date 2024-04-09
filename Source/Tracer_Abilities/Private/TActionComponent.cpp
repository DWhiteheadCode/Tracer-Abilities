// Fill out your copyright notice in the Description page of Project Settings.


#include "TActionComponent.h"

#include "TAction.h"
#include "GameplayTagContainer.h"

void UTActionComponent::AddAction(TSubclassOf<UTAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

	UTAction* NewAction = NewObject<UTAction>(this, ActionClass);
	if (ensure(NewAction))
	{
		Actions.Add(NewAction);
	}
}

bool UTActionComponent::StartActionByTag(FGameplayTag ActionTag)
{
	for (UTAction* Action : Actions)
	{
		if (Action && Action->IdentifierTag == ActionTag)
		{
			if (! Action->CanStart())
			{
				FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionTag.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);

				continue;
			}


			Action->StartAction();
			return true;
		}
	}

	return false;
}

bool UTActionComponent::StopActionByTag(FGameplayTag ActionTag)
{
	for (UTAction* Action : Actions)
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
