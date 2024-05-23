// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_PulseBomb.h"

#include "TActionComponent.h"

UTAction_PulseBomb::UTAction_PulseBomb()
{
	ActiveDuration = 0.5f;
	Cooldown = 20.f;
	StartingCharges = 0;
}

void UTAction_PulseBomb::StartAction_Implementation()
{
	Super::StartAction_Implementation();

	if (!ensureMsgf(ProjectileClass, TEXT("Must set projectile subclass in blueprints for Pulse Bomb Action")))
	{
		return;
	}

	APawn* const OwningPawn = Cast<APawn>(GetOwningComponent()->GetOwner());
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Coudln't start PulseBomb Action as OwningPawn was nullptr"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = OwningPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FRotator EyeRotation;
	FVector EyeLocation;
	OwningPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FTransform SpawnTM = FTransform(EyeRotation, EyeLocation);

	GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
}

FText UTAction_PulseBomb::GetNameText_Implementation() const
{
	return NSLOCTEXT("Actions", "PulseBomb_Name", "Pulse Bomb");
}