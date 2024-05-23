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
class UTHealthComponent;

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
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTActionComponent> ActionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTHealthComponent> HealthComp;

	// INPUT --------------------------------------------------------------
	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	// Move
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MovementAction;

	void Move(const FInputActionValue& Value);

	// Look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	void Look(const FInputActionValue& Value);

	// Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
	// Primary Ability
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PrimaryAbilityAction;

	// Starts an action in ActionComp that matches the tag PrimaryAbilityTag
	UFUNCTION()
	void StartPrimaryAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FGameplayTag PrimaryAbilityTag;

	// Secondary Ability 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SecondaryAbilityAction;

	// Starts an action in ActionComp that matches the tag SecondaryAbilityTag
	void StartSecondaryAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FGameplayTag SecondaryAbilityTag;

	// Ultimate Ability
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> UltimateAbilityAction;

	// Starts an action in ActionComp that matches the tag UltimateAbilityTag
	void StartUltimateAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FGameplayTag UltimateAbilityTag;

	// DEBUG --------------------------------------------------------------
	UFUNCTION(Exec)
	void HealSelf(int Amount = 100);
};
