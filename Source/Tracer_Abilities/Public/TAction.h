// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayTagContainer.h"

#include "TAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChargesChanged, int, RemainingCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownStarted, float, RemainingDuration);

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

	// MUST BE MANUALLY CALLED.
	virtual void BeginPlay();

	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag IdentifierTag;

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool CanStart() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction();

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StopAction();

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	int GetMaxCharges() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	int GetCurrentCharges() const;
		
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
	FText GetNameText() const;

	UPROPERTY(BlueprintAssignable)
	FOnChargesChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnCooldownStarted OnCooldownStarted; 

	UPROPERTY(BlueprintAssignable)
	FOnChargesChanged OnCooldownEnded;

protected:
	// Tags that will be applied to the UTActionComponent while this action is running
	UPROPERTY(EditAnywhere, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	// If the owning UTActionComponent has any of these tags, this action will not be able to start
	UPROPERTY(EditAnywhere, Category = "Tags")
	FGameplayTagContainer BlockedByTags;

	// Determines whether StartAction() should start a timer to call StopAction() after ActiveDuration seconds
	UPROPERTY(EditAnywhere, Category = "Action")
	bool bSetAutoEndTimer = true;

	UFUNCTION(BlueprintCallable, Category = "Action")
	UTActionComponent* GetOwningComponent() const;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bIsRunning = false;

	// ACTIVE DURATION ---------------------------------------
	UPROPERTY(EditAnywhere, Category = "Action")
	float ActiveDuration = 5.f;

	FTimerHandle TimerHandle_ActiveDuration;

	// COOLDOWN ----------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float Cooldown = 15.f;
	
	FTimerHandle TimerHandle_Cooldown;

	void StartCooldown();

	UFUNCTION()
	void OnCooldownEnd();

	UFUNCTION(BlueprintCallable, Category = "Action")
	float GetRemainingCooldown() const;

	// CHARGES -----------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	int MaxCharges = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	int StartingCharges = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	int CurrentCharges = 1;
};
