// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_Recall.h"

#include "TActionComponent.h"
#include "GameFramework/Character.h"
#include "THealthComponent.h"
#include "../Tracer_Abilities.h"

UTAction_Recall::UTAction_Recall()
{
	ActiveDuration = 1.5f;
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
		UE_LOG(LogTemp, Warning, TEXT("Recall BeginPlay had nullptr OwningCharacter. No recall data will be saved"));
		return;
	}

	// Save initial position
	PushRecallData();

	UWorld* World = OwningComp->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);

		World->GetTimerManager().SetTimer(
			TimerHandle_ClearOldRecallData, this, &UTAction_Recall::ClearOldRecallData, ClearInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall BeginPlay received nullptr UWorld from OwningComp. Recall data will not be saved/ cleared periodically"));
	}

	UTHealthComponent* const HealthComp = Cast<UTHealthComponent>(
		OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass()));
	if (HealthComp)
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &UTAction_Recall::OnOwningCharacterHealthChanged);
	}
}

void UTAction_Recall::PushRecallData()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::PushRecallData had nullptr OwningCharacter. New recall data won't be pushed."));
		return;
	}
	
	UWorld* const World = OwningCharacter->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::PushRecallData had nullptr World. New recall data won't be pushed."));
		return;
	}

	FRecallData RecallData;
	RecallData.Location = OwningCharacter->GetActorLocation();
	RecallData.Rotation = OwningCharacter->GetControlRotation();
	RecallData.GameTimeSeconds = World->GetTimeSeconds();
		
	if (UTHealthComponent* const HealthComp = Cast<UTHealthComponent>(
		OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass())))
	{
		RecallData.Health = HealthComp->GetHealth();
	}

	RecallDataArray.Insert(RecallData, 0);
}



void UTAction_Recall::ClearOldRecallData()
{
	UWorld* const World = OwningCharacter->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::ClearOldRecallData had nullptr World. Can't get current time, so data won't be cleared."));
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	
	for (int i = RecallDataArray.Num() - 1; i >= 0; i--)
	{
		const float RecallTime = RecallDataArray[i].GameTimeSeconds;

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

	UWorld* const World = OwningCharacter->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't start Recall as World was nullptr"));
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
	if (!bIsRunning)
	{
		return;
	}

	if (!OwningCharacter)
	{
		return;
	}

	UWorld* const World = OwningCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	float LerpValue = (CurrentTime - RecallStartTime) / ActiveDuration;

	const FVector CurrentPos = FMath::Lerp(RecallStartPos, RecallEndPos, LerpValue);
	const FRotator CurrentRot = FMath::Lerp(RecallStartRot, RecallEndRot, LerpValue);

	OwningCharacter->SetActorLocation(CurrentPos);
	if (AController* const Controller = OwningCharacter->GetController())
	{
		Controller->SetControlRotation(CurrentRot);
	}
}

void UTAction_Recall::StopAction_Implementation()
{
	if (OwningCharacter)
	{
		// Set the final transform as Tick might not have finished it
		OwningCharacter->SetActorLocation(RecallEndPos);
		if (AController* const Controller = OwningCharacter->GetController())
		{
			Controller->SetControlRotation(RecallEndRot);
		}

		UTHealthComponent* const HealthComp = Cast<UTHealthComponent>(OwningCharacter->GetComponentByClass(UTHealthComponent::StaticClass()));
		if (HealthComp)
		{
			if (MaxRecalledHealth > HealthComp->GetHealth())
			{
				HealthComp->SetHealth(MaxRecalledHealth);
			}
		}

		UWorld* const World = OwningCharacter->GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				TimerHandle_PushRecallData, this, &UTAction_Recall::PushRecallData, PushInterval, true);

			World->GetTimerManager().SetTimer(
				TimerHandle_ClearOldRecallData, this, &UTAction_Recall::ClearOldRecallData, ClearInterval, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Recall StopAction() had nullptr World. Recall data WON'T be pushed or cleared periodically"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Recall::StopAction had nullptr OwningCharacter. OwningCharacter health will not be updated, and timers won't be set (PushRecallData, MaxQueue)."));
	}

	RecallDataArray.Empty();

	// Prevents RecallDataArray from being empty until the next PushInterval
	PushRecallData();

	Super::StopAction_Implementation();
	OnActiveStateChanged(); // Super is used to change bIsRunning which this function needs
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

void UTAction_Recall::OnOwningCharacterHealthChanged(UTHealthComponent* const OwningComponent, int NewHealth, int ActualDelta)
{
	if (!bIsRunning)
	{
		PushRecallData();
	}
}

FText UTAction_Recall::GetNameText_Implementation() const
{
	return NSLOCTEXT("Actions", "Recall_Name", "Recall");
}