// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayTagContainer.h"

#include "TAction.generated.h"

class UTActionComponent;

/**
 * 
 */
UCLASS(Blueprintable)
class TRACER_ABILITIES_API UTAction : public UObject
{
	GENERATED_BODY()

public:
	UTAction();

	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag IdentifierTag;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStart();

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction();

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StopAction();

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;

	virtual void BeginPlay();

protected:
	UPROPERTY(EditAnywhere, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	UPROPERTY(EditAnywhere, Category = "Tags")
	FGameplayTagContainer BlockedByTags;

	UPROPERTY(EditAnywhere, Category = "Action")
	bool bSetAutoEndTimer;

	UFUNCTION(BlueprintCallable, Category = "Action")
	UTActionComponent* GetOwningComponent() const;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bIsRunning;

	// ACTIVE DURATION ---------------------------------------
	UPROPERTY(EditAnywhere, Category = "Action")
	float ActiveDuration;

	FTimerHandle TimerHandle_ActiveDuration;

	// COOLDOWN ----------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Action")
	float Cooldown;
	
	FTimerHandle TimerHandle_Cooldown;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bIsOnCooldown;

	UFUNCTION()
	void OnCooldownEnd();	
};
