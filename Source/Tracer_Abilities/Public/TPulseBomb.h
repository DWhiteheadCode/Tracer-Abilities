// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPulseBomb.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class TRACER_ABILITIES_API ATPulseBomb : public AActor
{
	GENERATED_BODY()

public:
	ATPulseBomb();

protected:
	void BeginPlay() override;

	// COMPONENTS ---------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;


	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComp;

	// BALANCE ------------------------------------------------
	
	UPROPERTY(EditAnywhere, Category = "Pulse Bomb")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, Category = "Pulse Bomb")
	float ExplosionDelay;

	// OTHER --------------------------------------------------
	FTimerHandle TimerHandle_Explosion;

	UFUNCTION(BlueprintCallable)
	void Explode();
};
