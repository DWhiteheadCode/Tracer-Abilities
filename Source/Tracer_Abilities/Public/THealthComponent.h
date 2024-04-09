// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "THealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, UTHealthComponent*, OwningComponent, int, NewHealth, int, ActualDelta);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRACER_ABILITIES_API UTHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetHealthMax() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(int Damage);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Health")
	int Health;

	UPROPERTY(EditAnywhere, Category = "Health")
	int HealthMax;

};
