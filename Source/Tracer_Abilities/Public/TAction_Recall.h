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
	float GameTimeSeconds;
};

class UTHealthComponent;

/**
 * 
 */
UCLASS()
class TRACER_ABILITIES_API UTAction_Recall : public UTAction, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UTAction_Recall();

	void StartAction_Implementation() override;

	void StopAction_Implementation() override;

	void BeginPlay() override;

	FText GetNameText_Implementation() const override;

	virtual void Tick(float DeltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Conditional;
	}

	virtual bool IsTickable() const override
	{
		return bIsRunning;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(TAction_Recall, STATGROUP_Tickables);
	}

protected:
	// Time between successive calls of PushRecallData()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recall")
	float PushInterval = 0.05f; 

	// Time between successive calls of ClearOldRecallData()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recall")
	float ClearInterval = 1.f;	

	// Total amount of time to rewind by (I.e. using this action will teleport the owner back to where they were
	// this many (TimeToRecall) seconds ago
	// 
	// Note: This is NOT the amount of time it takes for the rewind to complete after StartAction() is called.
	//     - That is UTAction::ActiveDuration.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recall")
	float TimeToRecall = 3.f;

	// Contains data from points in time that the recall *could* take the owner back to (or through)
	// The front element (index 0) is the most recent point in time, while the back element (index n) is the oldest.
	// 
	// Elements at the back of the array are removed once they are more than TimeToRecall seconds old (during the next ClearOldRecallData() call)
	// The backmost element is where the recall will position the owner once the ability ends, though all elements are used to check for the max Health value. 
	TArray<FRecallData> RecallDataArray;

	// Used to toggle certain characteristics based on whether this action is running or not. 
	// E.g.: Character's mesh is set to invisible and collision is disabled while the action is running.
	UFUNCTION()
	void OnActiveStateChanged();
	
	// Adds a new entry to RecallDataArray based on the character's current state (i.e.: saves OwningCharacter's current Location, Rotation and Health).
	UFUNCTION()
	void PushRecallData();

	FTimerHandle TimerHandle_PushRecallData;

	// Simply calls PushRecallData() whenever OwningCharacter's health changes to ensure no health change is missed between PushIntervals
	UFUNCTION()
	void OnOwningCharacterHealthChanged(UTHealthComponent* const OwningComponent, int NewHealth, int ActualDelta);

	// Removes all entries from RecallDataArray that are more than TimeToRecall seconds old. 
	UFUNCTION()
	void ClearOldRecallData();

	FTimerHandle TimerHandle_ClearOldRecallData;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTActionComponent> OwningComp;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACharacter> OwningCharacter;

	// CURRENT RECALL INFORMATION -------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "Recall")
	int MaxRecalledHealth = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Recall")
	FVector RecallStartPos;

	UPROPERTY(BlueprintReadOnly, Category = "Recall")
	FVector RecallEndPos;

	UPROPERTY(BlueprintReadOnly, Category = "Recall")
	FRotator RecallStartRot;

	UPROPERTY(BlueprintReadOnly, Category = "Recall")
	FRotator RecallEndRot;

	UPROPERTY(BlueprintReadOnly, Category = "Recall")
	float RecallStartTime;
};
