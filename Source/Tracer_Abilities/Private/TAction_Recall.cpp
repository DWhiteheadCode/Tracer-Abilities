// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_Recall.h"

#include "TActionComponent.h"
#include "GameFramework/Character.h"
#include "THealthComponent.h"
#include "../Tracer_Abilities.h"

UTAction_Recall::UTAction_Recall()
{
	ActiveDuration = 1.5;

	PushInterval = 0.05;
	ClearInterval = 1;
	TimeToRecall = 3;
	MaxRecalledHealth = 0;
}

void UTAction_Recall::BeginPlay()
{
	Super::BeginPlay();

	OwningComp = GetOwningComponent();
	if (ensure(OwningComp))
	{
		OwningCharacter = Cast<ACharacter>(OwningComp->GetOwner());
		if (ensure(OwningCharacter))
		{
			// Save initial position
			PushRecallData();

			UWorld* World = OwningComp->GetWorld();
			if (ensure(World))
			{
				World->GetTimerManager().SetTimer(
					TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);

				World->GetTimerManager().SetTimer(
					TimerHandle_ClearOldRecallData, this, &UTAction_Recall::ClearOldRecallData, ClearInterval, true);
			}

			UTHealthComponent* HealthComp = Cast<UTHealthComponent>(
				OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass()));
			if (HealthComp)
			{
				HealthComp->OnHealthChanged.AddDynamic(this, &UTAction_Recall::OnOwningCharacterHealthChanged);
			}
		}
	}
}

void UTAction_Recall::PushRecallData()
{
	if (ensure(OwningComp))
	{
		if (ensure(OwningCharacter))
		{
			FRecallData RecallData;
			RecallData.Location = OwningCharacter->GetActorLocation();
			RecallData.Rotation = OwningCharacter->GetControlRotation();

			UWorld* World = OwningCharacter->GetWorld();
			if (ensure(World))
			{
				RecallData.GameTimeSeconds = World->GetTimeSeconds();
			}

			if (UTHealthComponent* HealthComp = Cast<UTHealthComponent>(
				OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass())))
			{
				RecallData.Health = HealthComp->GetHealth();
			}

			RecallDataArray.Insert(RecallData, 0);
		}
	}
}



void UTAction_Recall::ClearOldRecallData()
{
	UWorld* World = OwningCharacter->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	
	for (int i = RecallDataArray.Num() - 1; i >= 0; i--)
	{
		float RecallTime = RecallDataArray[i].GameTimeSeconds;

		// If the element is more than TimeToRecall seconds old, remove it
		if (RecallTime < CurrentTime - TimeToRecall)
		{
			RecallDataArray.RemoveAt(i);
		}
		else
		{
			break; // All remaining elements are more recent than the current one
		}
	}
}



void UTAction_Recall::StartAction_Implementation()
{
	if (!ensure(OwningCharacter))
	{
		return;
	}

	// There should be at least 1 element, saved by a call to PushRecallData() from either the constructor or StopAction_Implementation()
	if (!ensure(RecallDataArray.Num() > 0))
	{
		return;
	}

	UWorld* World = OwningCharacter->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	World->GetTimerManager().ClearTimer(TimerHandle_PushRecallData);
	World->GetTimerManager().ClearTimer(TimerHandle_ClearOldRecallData);

	ClearOldRecallData();

	RecallStartTime = World->GetTimeSeconds();
	
	RecallStartPos = OwningCharacter->GetActorLocation();
	RecallStartRot = OwningCharacter->GetActorRotation();

	RecallEndPos = RecallDataArray.Last().Location;
	RecallEndRot = RecallDataArray.Last().Rotation;

	MaxRecalledHealth = 0;

	for (FRecallData& RecallData : RecallDataArray)
	{
		MaxRecalledHealth = FMath::Max(MaxRecalledHealth, RecallData.Health);
	}

	Super::StartAction_Implementation();
	OnActiveStateChanged(); // Must be called after Super::StartAction...(), otherwise bIsRunning won't be updated in time
}

void UTAction_Recall::Tick(float DeltaTime)
{
	if (!ensure(bIsRunning))
	{
		return;
	}

	if (!ensure(OwningCharacter))
	{
		return;
	}

	UWorld* World = OwningCharacter->GetWorld();
	if (ensure(World))
	{
		float CurrentTime = World->GetTimeSeconds();
		float LerpValue = (CurrentTime - RecallStartTime) / ActiveDuration;

		FVector CurrentPos = FMath::Lerp(RecallStartPos, RecallEndPos, LerpValue);
		FRotator CurrentRot = FMath::Lerp(RecallStartRot, RecallEndRot, LerpValue);

		OwningCharacter->SetActorLocation(CurrentPos);
		if (AController* Controller = OwningCharacter->GetController())
		{
			Controller->SetControlRotation(CurrentRot);
		}
	}
}

void UTAction_Recall::StopAction_Implementation()
{
	if (ensure(OwningCharacter))
	{
		// Set the final transform as Tick might not have finished it
		OwningCharacter->SetActorLocation(RecallEndPos);
		if (AController* Controller = OwningCharacter->GetController())
		{
			Controller->SetControlRotation(RecallEndRot);
		}	

		UTHealthComponent* HealthComp = Cast<UTHealthComponent>(OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass()));
		if (HealthComp)
		{		
			if (MaxRecalledHealth > HealthComp->GetHealth())
			{
				HealthComp->SetHealth(MaxRecalledHealth);
			}			
		}		

		UWorld* World = OwningCharacter->GetWorld();
		if (ensure(World))
		{
			World->GetTimerManager().SetTimer(
				TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);

			World->GetTimerManager().SetTimer(
				TimerHandle_ClearOldRecallData, this, &UTAction_Recall::ClearOldRecallData, ClearInterval, true);
		}		
	}	

	RecallDataArray.Empty();

	// Prevents RecallDataArray from being empty until the next PushInterval
	PushRecallData();

	Super::StopAction_Implementation();
	OnActiveStateChanged(); // Super is used to change bIsRunning which this function needs
}

void UTAction_Recall::OnActiveStateChanged()
{
	if (!ensure(OwningCharacter))
	{
		return;
	}

	OwningCharacter->SetActorEnableCollision(!bIsRunning);

	if (USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh())
	{
		CharacterMesh->SetVisibility(!bIsRunning);
	}

	if (APlayerController* Controller = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (bIsRunning)
		{
			OwningCharacter->DisableInput(Controller);
		}
		else
		{
			OwningCharacter->EnableInput(Controller);
		}
	}
}

FText UTAction_Recall::GetNameText_Implementation() const
{
	return NSLOCTEXT("Actions", "Recall_Name", "Recall");
}

void UTAction_Recall::OnOwningCharacterHealthChanged(UTHealthComponent* OwningComponent, int NewHealth, int ActualDelta)
{
	PushRecallData();
}
