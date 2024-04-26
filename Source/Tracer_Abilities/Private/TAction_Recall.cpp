// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_Recall.h"

#include "TActionComponent.h"
#include "GameFramework/Character.h"
#include "THealthComponent.h"
#include "../Tracer_Abilities.h"

DECLARE_CYCLE_STAT(TEXT("UpdateActorTransform"), STAT_UpdateActorTransform, STATGROUP_Tracer);
DECLARE_CYCLE_STAT(TEXT("UpdateActorTransform.NewSegment"), STAT_UpdateActorTransformNewSegment, STATGROUP_Tracer);

UTAction_Recall::UTAction_Recall()
{
	ActiveDuration = 1.5;

	PushInterval = 0.05;
	TransformUpdateInterval = 0.0167;
	TimeToRecall = 3;
	bQueueIsMaxSize = false;
	CurrentRecallIndex = -1;
	bSetAutoEndTimer = false; // Recall manually handles ending of action
}

void UTAction_Recall::BeginPlay()
{
	OwningComp = GetOwningComponent();
	if (ensure(OwningComp))
	{
		OwningCharacter = Cast<ACharacter>(OwningComp->GetOwner());
		if (ensure(OwningCharacter))
		{
			// Save initial position
			PushRecallData();

			OwningComp->GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);
			OwningComp->GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_MaxQueueSize, this, &UTAction_Recall::OnMaxQueueTimerEnd, TimeToRecall, false);
		}
	}
}


void UTAction_Recall::PushRecallData()
{
	if (bQueueIsMaxSize)
	{
		RecallDataArray.Pop(false); // Removes outdated data, which is not used for anything.
	}

	if (ensure(OwningComp))
	{
		if (ensure(OwningCharacter))
		{
			FRecallData RecallData;
			RecallData.Location = OwningCharacter->GetActorLocation();
			RecallData.Rotation = OwningCharacter->GetActorRotation();

			if (UTHealthComponent* HealthComp = Cast<UTHealthComponent>(
				OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass())))
			{
				RecallData.Health = HealthComp->GetHealth();
				RecallData.bSavedHealth = true;
			}

			RecallDataArray.Insert(RecallData, 0);
		}
	}
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

	Super::StartAction_Implementation();

	OnActiveStateChanged();

	OwningCharacter->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_MaxQueueSize);
	OwningCharacter->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PushRecallData);

	CurrentRecallIndex = 0;

	float SegmentDuration = ActiveDuration / RecallDataArray.Num();
	float StartTime = OwningCharacter->GetWorld()->GetTimeSeconds();
	
	FVector SegmentEndLocation = RecallDataArray[0].Location;
	FRotator SegmentEndRotation = RecallDataArray[0].Rotation;

	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "UpdateActorTransform",
		OwningCharacter->GetActorLocation(),
		SegmentEndLocation,
		OwningCharacter->GetActorRotation(),
		SegmentEndRotation,
		StartTime,
		SegmentDuration
		);
	
	//UE_LOG(LogTemp, Log, TEXT("STARTING RECALL:"));
	//UE_LOG(LogTemp, Log, TEXT("\tStart Time: %f"), StartTime);
	//UE_LOG(LogTemp, Log, TEXT("\tNum Segments: %i"), RecallDataArray.Num());
	//UE_LOG(LogTemp, Log, TEXT("\tSegment Duration: %f"), SegmentDuration);
	//UE_LOG(LogTemp, Log, TEXT("\tEstimated End Time: %f"), (StartTime + ActiveDuration));

	OwningCharacter->GetWorld()->GetTimerManager().SetTimer(TimerHandle_RecallSegment, Delegate, TransformUpdateInterval, true);
}

void UTAction_Recall::UpdateActorTransform(FVector SegmentStartPos, FVector SegmentEndPos, FRotator SegmentStartRot, FRotator SegmentEndRot, 	
	float SegmentStartTime, float SegmentDuration)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateActorTransform);

	if (!ensure(OwningCharacter))
	{
		return;
	}

	float CurrentTime = OwningCharacter->GetWorld()->GetTimeSeconds();
	float LerpValue = FMath::Clamp( ((CurrentTime - SegmentStartTime) / SegmentDuration), 0.f, 1.f);

	FVector CurrentPos = FMath::Lerp(SegmentStartPos, SegmentEndPos, LerpValue);
	FRotator CurrentRot = FMath::Lerp(SegmentStartRot, SegmentEndRot, LerpValue);

	OwningCharacter->SetActorLocation(CurrentPos);
	OwningCharacter->SetActorRotation(CurrentRot);

	// End of current segment
	if (LerpValue >= 1)
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateActorTransformNewSegment);

		UE_LOG(LogTemp, Log, TEXT("End recall segment. Current time: %f"), CurrentTime);

		CurrentRecallIndex++;

		// The last in the array segment has just finished
		if (!RecallDataArray.IsValidIndex(CurrentRecallIndex))
		{
			CurrentRecallIndex = -1;
			OwningCharacter->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_RecallSegment);

			StopAction();

			return;
		}

		FVector NextSegmentStartPos = SegmentEndPos;
		FVector NextSegmentEndPos = RecallDataArray[CurrentRecallIndex].Location;

		FRotator NextSegmentStartRot = SegmentEndRot;
		FRotator NextSegmentEndRot = RecallDataArray[CurrentRecallIndex].Rotation;

		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "UpdateActorTransform",
			NextSegmentStartPos,
			NextSegmentEndPos,
			NextSegmentStartRot,
			NextSegmentEndRot,
			CurrentTime,
			SegmentDuration
		);

		OwningCharacter->GetWorld()->GetTimerManager().SetTimer(TimerHandle_RecallSegment, Delegate, TransformUpdateInterval, true);
	}
}


void UTAction_Recall::StopAction_Implementation()
{
	if (!ensure( RecallDataArray.Num() > 0 ))
	{		
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("RECALL ENDING"));


	if (ensure(OwningCharacter))
	{
		//UE_LOG(LogTemp, Log, TEXT("\tEnd Time: %f"), (OwningCharacter->GetWorld()->GetTimeSeconds()));

		FRecallData FinalRecallData = RecallDataArray.Pop();

		OwningCharacter->SetActorLocation( FinalRecallData.Location );

		if (FinalRecallData.bSavedHealth)
		{
			UTHealthComponent* HealthComp = Cast<UTHealthComponent>(OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass()));
			if (ensure(HealthComp))
			{
				// Only update health if recalled health is more than current health
				if ( FinalRecallData.Health > HealthComp->GetHealth() )
				{
					HealthComp->SetHealth(FinalRecallData.Health);
				}
			}
		}

		OwningCharacter->GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_MaxQueueSize, this, &UTAction_Recall::OnMaxQueueTimerEnd, TimeToRecall, false);

		OwningComp->GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);
	}	

	RecallDataArray.Empty();
	bQueueIsMaxSize = false;

	// Prevents RecallDataArray from being empty until the next PushInterval
	PushRecallData();

	Super::StopAction_Implementation();

	OnActiveStateChanged(); // Super is used to change bIsRunning
}

void UTAction_Recall::OnMaxQueueTimerEnd()
{
	bQueueIsMaxSize = true;
}