// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPulseBomb.generated.h"

class UStaticMeshComponent;

UCLASS()
class TRACER_ABILITIES_API ATPulseBomb : public AActor
{
	GENERATED_BODY()

public:
	ATPulseBomb();

protected:
	void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Pulse Bomb")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, Category = "Pulse Bomb")
	float ExplosionDelay;

	FTimerHandle TimerHandle_Explosion;

	UFUNCTION(BlueprintCallable)
	void Explode();
};
