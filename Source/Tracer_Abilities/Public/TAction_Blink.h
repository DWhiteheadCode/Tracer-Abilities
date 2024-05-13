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

	// Returns the location that CharacterToTeleport will be teleported to after using this blink.
	// Uses their GetLastMovementInput() to determine the blink direction. 
	// A capsule is swept in this direction. If it hits terrain, the capsule's location at the point of the hit is returned.
	//     If no hit occurs, MaxBlinkDistance is used to determine the location
	UFUNCTION(BlueprintCallable, Category = "Blink")
	FVector GetTeleportDestination(ACharacter* CharacterToTeleport);

	FText GetNameText_Implementation() const override;
};
