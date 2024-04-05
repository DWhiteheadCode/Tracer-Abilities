// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TCharacter.generated.h"

class UCameraComponent;

UCLASS()
class TRACER_ABILITIES_API ATCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UCameraComponent* CameraComp;

};
