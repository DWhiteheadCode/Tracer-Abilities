// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_Blink.h"

#include "TActionComponent.h"
#include "TCharacter.h"
#include "Components/CapsuleComponent.h"

UTAction_Blink::UTAction_Blink()
{
	MaxBlinkDistance = 250;
}

void UTAction_Blink::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	UTActionComponent* OwningComp = GetOwningComponent();
	if (!ensure(OwningComp))
	{
		return;
	}

	ATCharacter* OwningCharacter = Cast<ATCharacter>(OwningComp->GetOwner());
	if (!ensure(OwningCharacter))
	{
		return;
	}

	FVector Destination = GetTeleportDestination(OwningCharacter);

	OwningCharacter->TeleportTo(Destination, OwningCharacter->GetActorRotation());

	StopAction(Instigator);
}

FVector UTAction_Blink::GetTeleportDestination(ATCharacter* CharacterToTeleport)
{
	if (!ensure(CharacterToTeleport))
	{
		return FVector::Zero();
	}

	FHitResult HitResult;

	FVector StartLocation = CharacterToTeleport->GetActorLocation();
	FVector InputDirection = CharacterToTeleport->GetLastMovementInputVector();

	if (InputDirection == FVector::Zero())
	{
		InputDirection = CharacterToTeleport->GetActorForwardVector();
	}

	FVector EndLocation = StartLocation + (InputDirection * MaxBlinkDistance);

	DrawDebugSphere(GetWorld(), StartLocation, 2, 8, FColor::Green, false, 5.0f);
	DrawDebugSphere(GetWorld(), EndLocation, 2, 8, FColor::Blue, false, 5.0f);

	FCollisionObjectQueryParams Params;
	Params.AddObjectTypesToQuery(ECC_WorldStatic);
	Params.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionShape Shape;

	UCapsuleComponent* CharacterCapsule = CharacterToTeleport->GetCapsuleComponent();
	if (ensure(CharacterCapsule))
	{
		Shape.SetCapsule(CharacterCapsule->GetScaledCapsuleRadius(), CharacterCapsule->GetScaledCapsuleHalfHeight());

		FColor DebugCapsuleColor = FColor::White;

		if (GetWorld()->SweepSingleByObjectType(HitResult, StartLocation, EndLocation, FQuat::Identity, Params, Shape))
		{
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 2, 8, FColor::Red, false, 5.0f);

			FVector ReversedSweepDirection = StartLocation - EndLocation;
			ReversedSweepDirection.Normalize();

			// Set the location such that the edge of the character's capsule will *just* touch the impact point
			EndLocation = HitResult.ImpactPoint + (ReversedSweepDirection * CharacterCapsule->GetScaledCapsuleRadius());
			EndLocation.Z = StartLocation.Z; // Ignore vertical change based on where the collision occurred

			DebugCapsuleColor = FColor::Red;
		}

		DrawDebugCapsule(GetWorld(), EndLocation, CharacterCapsule->GetScaledCapsuleHalfHeight(),
			CharacterCapsule->GetScaledCapsuleRadius(), FQuat::Identity, DebugCapsuleColor, false, 5.0f);
	}

	return EndLocation;
}


