// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TAction.h"
#include "TAction_PulseBomb.generated.h"

class ATPulseBomb;

/**
 * 
 */
UCLASS()
class TRACER_ABILITIES_API UTAction_PulseBomb : public UTAction
{
	GENERATED_BODY()
	
public:
	void StartAction_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Pulse Bomb")
	TSubclassOf<AActor> ProjectileClass;
};