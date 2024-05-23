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
	// Delay between light flashes, as well as the duration of each light flash
	UPROPERTY(EditAnywhere, Category = "Visual")
	float LightFlashDelay = 0.1f;

	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ToggleLight();

	FTimerHandle TimerHandle_LightToggle;

	// BALANCE ------------------------------------------------	
	// Radius used for the collision sphere that will "stick" the bomb to a surface/ player
	UPROPERTY(EditAnywhere, Category = "Balance")
	float StickRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Balance")
	float ExplosionDelay = 2.f;

	// Damage dealt to targets that are MinDamage_Range far away from the bomb when it explodes 
	UPROPERTY(EditAnywhere, Category = "Balance")
	int MinDamage = 5;

	// Damage dealt to targets that are MaxDamage_Range far away from the bomb (or closer) when it explodes 
	UPROPERTY(EditAnywhere, Category = "Balance")
	int MaxDamage = 100;

	// This is the furthest distance from the bomb that MaxDamage will be dealt.
	// I.e. This is the distance BEFORE damage falloff starts to occur
	// MaxDamage_Range must be <= MinDamage_Range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance")
	float MaxDamage_Range = 50.f;

	// This is the distance from the bomb at which MinDamage will be dealt. 
	// Actors that are further away from the bomb than this distance will not be damaged at all.
	//     I.e. This is the radius of the explosion.
	// MinDamage_Range must be >= MaxDamage_Range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance")
	float MinDamage_Range = 300.f;

	// OTHER --------------------------------------------------
	FTimerHandle TimerHandle_Explosion;

	UFUNCTION(BlueprintCallable)
	void Explode();

	UFUNCTION(BlueprintCallable)
	int CalculateDamage(AActor* const ActorToDamage) const;

	// Checks if terrain is blocking the path between the bomb and ActorToDamage. 
	// Returns true if the path is blocked, and false if it is not. 
	UFUNCTION(BlueprintCallable)
	bool IsDamagePathBlocked(AActor* const ActorToDamage) const;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
