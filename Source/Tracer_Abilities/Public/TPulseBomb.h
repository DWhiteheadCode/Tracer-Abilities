// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPulseBomb.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UProjectileMovementComponent;
class UPointLightComponent;

UCLASS()
class TRACER_ABILITIES_API ATPulseBomb : public AActor
{
	GENERATED_BODY()

public:
	ATPulseBomb();

protected:
	void BeginPlay() override;

	void PostInitializeComponents() override;

	// COMPONENTS ---------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPointLightComponent> LightComp;

	// VISUAL -------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Visual")
	float LightFlashDelay;

	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ToggleLight();

	FTimerHandle TimerHandle_LightToggle;

	// BALANCE ------------------------------------------------	
	UPROPERTY(EditAnywhere, Category = "Balance")
	float StickRadius;

	UPROPERTY(EditAnywhere, Category = "Balance")
	float ExplosionDelay;

	UPROPERTY(EditAnywhere, Category = "Balance")
	int MinDamage;

	UPROPERTY(EditAnywhere, Category = "Balance")
	int MaxDamage;

	// This is the furthest distance from the bomb that MaxDamage will be dealt.
	// I.e. This is the distance BEFORE damage falloff starts to occur
	// MaxDamage_Range must be <= MinDamage_Range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance")
	float MaxDamage_Range;

	// This is the distance from the bomb at which MinDamage will be dealt. 
	// Actors that are further away from the bomb than this distance will not be damaged at all.
	// I.e. This is the radius of the explosion.
	// MinDamage_Range must be >= MaxDamage_Range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance")
	float MinDamage_Range;

	// OTHER --------------------------------------------------
	FTimerHandle TimerHandle_Explosion;

	UFUNCTION(BlueprintCallable)
	void Explode();

	UFUNCTION(BlueprintCallable)
	int CalculateDamage(AActor* ActorToDamage);

	UFUNCTION(BlueprintCallable)
	bool IsDamagePathBlocked(AActor* ActorToDamage);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
