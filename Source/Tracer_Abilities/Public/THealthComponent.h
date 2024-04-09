// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, UTHealthComponent*, OwningComponent, float, NewHealth, float, ActualDelta);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRACER_ABILITIES_API UTHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthMax() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float Damage);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Health")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthMax;

};
