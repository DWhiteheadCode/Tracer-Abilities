// Fill out your copyright notice in the Description page of Project Settings.


#include "TCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "TActionComponent.h"

ATCharacter::ATCharacter()
{
	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(RootComponent);

	ActionComp = CreateDefaultSubobject<UTActionComponent>("Action Component");
}

void ATCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem 
			= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}
}

void ATCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ATCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ATCharacter::Jump);

		// Abilities
		EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Started, this, &ATCharacter::StartPrimaryAbility);
		EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Started, this, &ATCharacter::StartSecondaryAbility);
		EnhancedInputComponent->BindAction(UltimateAbilityAction, ETriggerEvent::Started, this, &ATCharacter::StartUltimateAbility);
	}
}

void ATCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();

	if (GetController())
	{
		FVector Forward = GetActorForwardVector();
		AddMovementInput(Forward, MoveValue.X);

		FVector Right = GetActorRightVector();
		AddMovementInput(Right, MoveValue.Y);
	}
}

void ATCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}

void ATCharacter::StartPrimaryAbility()
{
	ActionComp->StartActionByTag(this, PrimaryAbilityTag);
}

void ATCharacter::StartSecondaryAbility()
{
	ActionComp->StartActionByTag(this, SecondaryAbilityTag);
}

void ATCharacter::StartUltimateAbility()
{
	ActionComp->StartActionByTag(this, UltimateAbilityTag);
}
