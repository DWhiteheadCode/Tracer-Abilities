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
	ActiveDuration = 1.5f;
	bSetAutoEndTimer = false; // Recall manually handles ending of action
}

void UTAction_Recall::BeginPlay()
{
	Super::BeginPlay();

	OwningComp = GetOwningComponent();
	if (!OwningComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall BeginPlay had nullptr OwningComp. OwningCharacter will remain nullptr, and PushRecallData and MaxQueue timers will not start."));
		return;
	}

	OwningCharacter = Cast<ACharacter>(OwningComp->GetOwner());
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall BeginPlay had nullptr OwningCharacter. PushRecallData and MaxQueue timers will not start."));
		return;
	}

	// Save initial position
	PushRecallData();

	OwningComp->GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);
	OwningComp->GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_MaxQueueSize, this, &UTAction_Recall::OnMaxQueueTimerEnd, TimeToRecall, false);
}




void UTAction_Recall::PushRecallData()
{
	if (bQueueIsMaxSize)
	{
		RecallDataArray.Pop(false); // Removes outdated data, which is not used for anything.
	}
	
	if (OwningCharacter)
	{
		FRecallData RecallData;
		RecallData.Location = OwningCharacter->GetActorLocation();
		RecallData.Rotation = OwningCharacter->GetControlRotation();

		if (UTHealthComponent* const HealthComp = Cast<UTHealthComponent>(
			OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass())))
		{
			RecallData.Health = HealthComp->GetHealth();
		}

		RecallDataArray.Insert(RecallData, 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::PushRecallData had nullptr OwningCharacter"));
	}
}

void UTAction_Recall::OnActiveStateChanged()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::OnActiveStateChanged had nullptr OwningCharacter"));
		return;
	}

	OwningCharacter->SetActorEnableCollision(!bIsRunning);

	if (USkeletalMeshComponent* const CharacterMesh = OwningCharacter->GetMesh())
	{
		CharacterMesh->SetVisibility(!bIsRunning);
	}

	if (APlayerController* const Controller = Cast<APlayerController>(OwningCharacter->GetController()))
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
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't start Recall as OwningCharacter was nullptr"));
		return;
	}

	// There should be at least 1 element, saved by a call to PushRecallData() from either the constructor or StopAction_Implementation()
	if (RecallDataArray.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't start Recall as RecallDataArray was empty"));
		return;
	}

	Super::StartAction_Implementation();

	OnActiveStateChanged();

	OwningCharacter->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_MaxQueueSize);
	OwningCharacter->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PushRecallData);

	CurrentRecallIndex = 0;

	const float SegmentDuration = ActiveDuration / RecallDataArray.Num();
	const float StartTime = OwningCharacter->GetWorld()->GetTimeSeconds();
	
	const FVector SegmentEndLocation = RecallDataArray[0].Location;
	const FRotator SegmentEndRotation = RecallDataArray[0].Rotation;

	MaxRecalledHealth = RecallDataArray[0].Health;

	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "UpdateActorTransform",
		OwningCharacter->GetActorLocation(),
		SegmentEndLocation,
		OwningCharacter->GetActorRotation(),
		SegmentEndRotation,
		StartTime,
		SegmentDuration
		);

	OwningCharacter->GetWorld()->GetTimerManager().SetTimer(TimerHandle_RecallSegment, Delegate, TransformUpdateInterval, true);
}

void UTAction_Recall::UpdateActorTransform(const FVector SegmentStartPos, const FVector SegmentEndPos, const FRotator SegmentStartRot, 
	const FRotator SegmentEndRot, const float SegmentStartTime, const float SegmentDuration)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateActorTransform);

	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::UpdateActorTransform had nullptr OwningCharacter"));
		return;
	}

	const float CurrentTime = OwningCharacter->GetWorld()->GetTimeSeconds();
	const float LerpValue = FMath::Clamp( ((CurrentTime - SegmentStartTime) / SegmentDuration), 0.f, 1.f);

	const FVector CurrentPos = FMath::Lerp(SegmentStartPos, SegmentEndPos, LerpValue);
	const FRotator CurrentRot = FMath::Lerp(SegmentStartRot, SegmentEndRot, LerpValue);

	OwningCharacter->SetActorLocation(CurrentPos);
	if (AController* const Controller = OwningCharacter->GetController())
	{
		Controller->SetControlRotation(CurrentRot);
	}	

	// End of current segment
	if (LerpValue >= 1.f)
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateActorTransformNewSegment);

		CurrentRecallIndex++;

		// The last segment has just finished (i.e. the character is at the final location/rotation from this recall)
		if (!RecallDataArray.IsValidIndex(CurrentRecallIndex))
		{
			CurrentRecallIndex = -1;
			OwningCharacter->GetWorld()->GetTimerManager().ClearTimer(TimerHandle_RecallSegment);

			StopAction();

			return;
		}
				
		MaxRecalledHealth = FMath::Max(MaxRecalledHealth, RecallDataArray[CurrentRecallIndex].Health);
		
		const FVector NextSegmentStartPos = SegmentEndPos;
		const FVector NextSegmentEndPos = RecallDataArray[CurrentRecallIndex].Location;

		const FRotator NextSegmentStartRot = SegmentEndRot;
		const FRotator NextSegmentEndRot = RecallDataArray[CurrentRecallIndex].Rotation;

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
	if (OwningCharacter)
	{
		UTHealthComponent* const HealthComp = Cast<UTHealthComponent>(OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass()));
		if (HealthComp)
		{		
			if (MaxRecalledHealth > HealthComp->GetHealth())
			{
				HealthComp->SetHealth(MaxRecalledHealth);
			}			
		}		

		OwningCharacter->GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_MaxQueueSize, this, &UTAction_Recall::OnMaxQueueTimerEnd, TimeToRecall, false);

		OwningComp->GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);
	}	
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::StopAction had nullptr OwningCharacter. OwningCharacter health will not be updated, and timers won't be set (PushRecallData, MaxQueue)."));
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

FText UTAction_Recall::GetNameText_Implementation() const
{
	return NSLOCTEXT("Actions", "Recall_Name", "Recall");
}