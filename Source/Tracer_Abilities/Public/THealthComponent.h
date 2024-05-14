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

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyHeal(int Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealth(int NewHealth);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Health")
	int Health = 100;

	UPROPERTY(EditAnywhere, Category = "Health")
	int HealthMax = 100;

};
