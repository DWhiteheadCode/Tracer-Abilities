// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameplayTagContainer.h"

#include "TActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionAdded, UTAction*, ChangedAction);

class UTAction;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRACER_ABILITIES_API UTActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddAction(TSubclassOf<UTAction> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByTag(FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByTag(FGameplayTag ActionTag);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;	

	UPROPERTY(BlueprintAssignable)
	FOnActionAdded OnActionAdded;

protected:
	// An array containing all actions in this ActionComponent
	UPROPERTY(BlueprintReadOnly)
	TArray< TObjectPtr<UTAction> > Actions;
	
	// An array of actions that will be added to this ActionComponent when BeginPlay() is called
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<UTAction>> DefaultActions;

};
