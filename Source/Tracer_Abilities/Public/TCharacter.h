// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputActionValue.h"
#include "GameplayTagContainer.h"

#include "TCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UTActionComponent;

UCLASS()
class TRACER_ABILITIES_API ATCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	// COMPONENTS ---------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTActionComponent* ActionComp;

	// INPUT --------------------------------------------------------------
	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultInputMappingContext;

	// Move
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MovementAction;

	void Move(const FInputActionValue& Value);

	// Look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	void Look(const FInputActionValue& Value);

	// Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	
	// Primary Ability
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PrimaryAbilityAction;

	UFUNCTION()
	void StartPrimaryAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FGameplayTag PrimaryAbilityTag;

	// Secondary Ability 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SecondaryAbilityAction;

	void StartSecondaryAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FGameplayTag SecondaryAbilityTag;

	// Ultimate Ability
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* UltimateAbilityAction;

	void StartUltimateAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FGameplayTag UltimateAbilityTag;

};
