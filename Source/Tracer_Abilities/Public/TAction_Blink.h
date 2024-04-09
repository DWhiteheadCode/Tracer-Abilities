// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TAction.h"
#include "TAction_Blink.generated.h"

class ACharacter;

/**
 * 
 */
UCLASS()
class TRACER_ABILITIES_API UTAction_Blink : public UTAction
{
	GENERATED_BODY()
	
public:
	UTAction_Blink();

	UPROPERTY(EditDefaultsOnly, Category="Blink")
	float MaxBlinkDistance;

	UFUNCTION()
	void StartAction_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Blink")
	FVector GetTeleportDestination(ACharacter* CharacterToTeleport);
};
