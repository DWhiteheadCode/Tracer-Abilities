// Fill out your copyright notice in the Description page of Project Settings.


#include "TCharacter.h"

#include "Camera/CameraComponent.h"

ATCharacter::ATCharacter()
{
	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(RootComponent);
}

void ATCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

