// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TAction.h"
#include "TAction_Recall.generated.h"

USTRUCT()
struct FRecallData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	int Health;

	UPROPERTY()
	bool bSavedHealth;
};

/**
 * 
 */
UCLASS()
class TRACER_ABILITIES_API UTAction_Recall : public UTAction
{
	GENERATED_BODY()
	
public:
	UTAction_Recall();

	void StartAction_Implementation() override;

	void StopAction_Implementation() override;

	void BeginPlay() override;


protected:
	// Contains data from points in time that the recall *could* take the owner back to (or through)
	// The front element (index 0) is the most recent point in time, while the back element (index n) is the oldest.
	// 
	// Elements at the back of the array are removed once they are more than TimeToRecall seconds old (during the next PushRecallData() update)
	// The backmost element is where the recall will position the owner, though all elements (specifically their Location) are used to "rewind" back to that point
	TArray<FRecallData> RecallDataArray;

	// Time between successive calls of PushRecallData()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recall")
	float PushInterval; 

	// Time between updates to actor's position during the use of recall
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recall")
	float TransformUpdateInterval;

	// Total amount of time to rewind by (I.e. using this action will teleport the owner back to where they were
	// this many seconds ago
	// 
	// Note: This is NOT the amount of time it takes for the rewind to complete after StartAction() is called.
	//     - That is ActiveDuration.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recall")
	float TimeToRecall;

	bool bQueueIsMaxSize;

	FTimerHandle TimerHandle_MaxQueueSize;

	FTimerHandle TimerHandle_PushRecallData;

	FTimerHandle TimerHandle_RecallSegment;

	UFUNCTION()
	void OnActiveStateChanged();

	UFUNCTION()
	void OnMaxQueueTimerEnd();
	
	UFUNCTION()
	void PushRecallData();

	UFUNCTION()
	void UpdateActorTransform(FVector SegmentStartPos, FVector SegmentEndPos, 
		FRotator SegmentStartRot, FRotator SegmentEndRot,
		float SegmentStartTime, float SegmentDuration);

	UPROPERTY(BlueprintReadOnly)
	UTActionComponent* OwningComp;

	UPROPERTY(BlueprintReadOnly)
	ACharacter* OwningCharacter;

	int CurrentRecallIndex;
};
