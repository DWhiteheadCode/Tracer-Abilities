// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_PulseBomb.h"

#include "TActionComponent.h"

void UTAction_PulseBomb::StartAction_Implementation()
{
	Super::StartAction_Implementation();

	if (!ensureMsgf(ProjectileClass, TEXT("Must set projectile subclass in blueprints for Pulse Bomb Action")))
	{
		return;
	}

	APawn* OwningPawn = Cast<APawn>(GetOwningComponent()->GetOwner());
	if (!ensure(OwningPawn))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = OwningPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FRotator EyeRotation;
	FVector EyeLocation;
	OwningPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FTransform SpawnTM = FTransform(EyeRotation, EyeLocation);

	GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
}
