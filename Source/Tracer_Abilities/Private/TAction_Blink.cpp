// Fill out your copyright notice in the Description page of Project Settings.


#include "TAction_Blink.h"

#include "TActionComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

static TAutoConsoleVariable<bool> CVarBlinkDebugLines(TEXT("t.BlinkDebug"), false, TEXT("Draw debug dots and capsules for blinks"), ECVF_Cheat);

UTAction_Blink::UTAction_Blink()
{
	ActiveDuration = 0.1f;
	Cooldown = 10.f;
	
	MaxCharges = 3;
	StartingCharges = 3;
}

void UTAction_Blink::StartAction_Implementation()
{
	Super::StartAction_Implementation();

	UTActionComponent* const OwningComp = GetOwningComponent();
	if (!OwningComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start Blink Action, but it didn't have OwningComp"));
		return;
	}

	ACharacter* const OwningCharacter = Cast<ACharacter>(OwningComp->GetOwner());
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to start Blink Action, but it didn't have OwningCharacter"));
		return;
	}

	const FVector Destination = GetTeleportDestination(OwningCharacter);

	// Manually update the character's velocity to prevent carrying momentum in the opposite direction of the blink
	if (UCharacterMovementComponent* const MoveComp = OwningCharacter->GetCharacterMovement())
	{
		FVector OldVelocity = MoveComp->Velocity;
		OldVelocity.Z = 0; // Ignore vertical component
		const float Speed = OldVelocity.Size();

		FVector InputDirection = OwningCharacter->GetLastMovementInputVector();
		InputDirection.Normalize();

		MoveComp->Velocity = InputDirection * Speed;
	}

	OwningCharacter->TeleportTo(Destination, OwningCharacter->GetActorRotation());
}

FVector UTAction_Blink::GetTeleportDestination(ACharacter* const CharacterToTeleport)
{
	if (!CharacterToTeleport)
	{
		UE_LOG(LogTemp, Warning, TEXT("Blink tried to GetTeleportDestination for nullptr Character"));
		return FVector::Zero();
	}

	FHitResult HitResult;

	const FVector StartLocation = CharacterToTeleport->GetActorLocation();
	FVector InputDirection = CharacterToTeleport->GetLastMovementInputVector();
	InputDirection.Normalize();

	// If no movement direction is being input, use the ActorForwardVector as input direction
	if (InputDirection == FVector::Zero())
	{
		InputDirection = CharacterToTeleport->GetActorForwardVector();
	}

	FVector EndLocation = StartLocation + (InputDirection * MaxBlinkDistance);

	if (CVarBlinkDebugLines.GetValueOnGameThread())
	{
		DrawDebugSphere(GetWorld(), StartLocation, 2, 8, FColor::Green, false, 5.0f);
		DrawDebugSphere(GetWorld(), EndLocation, 2, 8, FColor::Blue, false, 5.0f);
	}

	FCollisionObjectQueryParams Params;
	Params.AddObjectTypesToQuery(ECC_WorldStatic);
	Params.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionShape Shape;

	UCapsuleComponent* const CharacterCapsule = CharacterToTeleport->GetCapsuleComponent();
	if (CharacterCapsule)
	{
		Shape.SetCapsule(CharacterCapsule->GetScaledCapsuleRadius(), CharacterCapsule->GetScaledCapsuleHalfHeight());

		FColor DebugCapsuleColor = FColor::White;

		// Sweep a capsule of the same size as the character's in the input direction
		// If it hits terrain, calculate the destination locaction based on the hit location
		if (GetWorld()->SweepSingleByObjectType(HitResult, StartLocation, EndLocation, FQuat::Identity, Params, Shape))
		{
			if (CVarBlinkDebugLines.GetValueOnGameThread())
			{
				DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 2, 8, FColor::Red, false, 5.0f);
			}			

			FVector ReversedSweepDirection = StartLocation - EndLocation;
			ReversedSweepDirection.Normalize();

			// Set the location such that the edge of the character's capsule will *just* touch the impact point
			EndLocation = HitResult.ImpactPoint + (ReversedSweepDirection * CharacterCapsule->GetScaledCapsuleRadius());
			EndLocation.Z = StartLocation.Z; // Ignore vertical change based on where the collision occurred

			DebugCapsuleColor = FColor::Red;
		}

		if (CVarBlinkDebugLines.GetValueOnGameThread())
		{
			DrawDebugCapsule(GetWorld(), EndLocation, CharacterCapsule->GetScaledCapsuleHalfHeight(),
				CharacterCapsule->GetScaledCapsuleRadius(), FQuat::Identity, DebugCapsuleColor, false, 5.0f);
		}		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Blink couldn't perform sweep as CharacterToTeleport didn't have a UCapsuleComponent. Using MaxBlinkDistance to calculate EndLocation."));
	}

	return EndLocation;
}

FText UTAction_Blink::GetNameText_Implementation() const
{
	return NSLOCTEXT("Actions", "Blink_Name", "Blink");
}