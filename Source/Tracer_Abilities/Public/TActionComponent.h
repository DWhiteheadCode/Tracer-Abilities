// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameplayTagContainer.h"

#include "TActionComponent.generated.h"

class UTAction;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRACER_ABILITIES_API UTActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTActionComponent();

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddAction(TSubclassOf<UTAction> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByTag(AActor* Instigator, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByTag(AActor* Instigator, FGameplayTag ActionTag);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;

	void BeginPlay() override;

protected:
	UPROPERTY()
	TArray<UTAction*> Actions;
	
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<UTAction>> DefaultActions;

};
