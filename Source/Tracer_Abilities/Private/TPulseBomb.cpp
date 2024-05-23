// Fill out your copyright notice in the Description page of Project Settings.


#include "TPulseBomb.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "THealthComponent.h"
#include "Components/PointLightComponent.h"

static TAutoConsoleVariable<bool> CVarPulseBombDebugLines(TEXT("t.PulseBombDebug"), false, TEXT("Draw debug lines for pulse bombs"), ECVF_Cheat);

ATPulseBomb::ATPulseBomb()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionProfileName("NoCollision");
	RootComponent = MeshComp;

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComp");
	ProjectileMovementComp->InitialSpeed = 1250.f;
	ProjectileMovementComp->ProjectileGravityScale = 3.f;

	CollisionSphereComp = CreateDefaultSubobject<USphereComponent>("CollisionSphereComp");
	CollisionSphereComp->SetupAttachment(RootComponent);
	CollisionSphereComp->SetSphereRadius(StickRadius);
	CollisionSphereComp->SetCollisionProfileName("Projectile");
	CollisionSphereComp->SetGenerateOverlapEvents(true);

	LightComp = CreateDefaultSubobject<UPointLightComponent>("LightComp");
	LightComp->SetupAttachment(RootComponent);
	LightComp->SetLightColor(FColor::Blue);
	LightComp->SetSourceRadius(StickRadius);
}

void ATPulseBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CollisionSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ATPulseBomb::OnBeginOverlap);
}

void ATPulseBomb::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_LightToggle, this, &ATPulseBomb::ToggleLight, LightFlashDelay, true);
	GetWorldTimerManager().SetTimer(TimerHandle_Explosion, this, &ATPulseBomb::Explode, ExplosionDelay, false);
}

void ATPulseBomb::Explode()
{
	UE_LOG(LogTemp, Log, TEXT("Pulse bomb exploded"));

	if (CVarPulseBombDebugLines.GetValueOnGameThread())
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), MinDamage_Range, 16, FColor::Red, false, 3.0f, 2, 1.0f);
		DrawDebugSphere(GetWorld(), GetActorLocation(), MaxDamage_Range, 16, FColor::Orange, false, 3.0f, 2, 1.0f);
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	const TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> NearbyActors;

	UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), MinDamage_Range, ObjectTypes, nullptr, ActorsToIgnore, NearbyActors);

	for (AActor* const NearbyActor : NearbyActors)
	{
		if (NearbyActor)
		{
			if (UTHealthComponent* const HealthComp = Cast<UTHealthComponent>(NearbyActor->GetComponentByClass(UTHealthComponent::StaticClass())))
			{
				const int Damage = CalculateDamage(NearbyActor);
					
				if (Damage > 0)
				{
					HealthComp->ApplyDamage(Damage);
				}				
			}
		}
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_LightToggle);
	LightComp->SetVisibility(false);

	MeshComp->SetVisibility(false);
	SetActorEnableCollision(false);
	SetLifeSpan(2.f);
}

int ATPulseBomb::CalculateDamage(AActor* const ActorToDamage) const
{
	if (! ensureMsgf(MinDamage_Range > 0.f, TEXT("MinDamage_Range must be > 0")))
	{
		return 0;
	}

	if (!ensureMsgf(MaxDamage_Range > 0.f, TEXT("MaxDamage_Range must be > 0")))
	{
		return 0;
	}

	// Without this check, more damage could be dealt the further away the target is from the bomb 
	if (!ensureMsgf(MinDamage_Range >= MaxDamage_Range, TEXT("MinDamage_Range must be >= MaxDamage_Range")))
	{
		return 0;
	}

	if (!ActorToDamage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PulseBomb tried to calculate damage for nullptr ActorToDamage"));
		return 0;
	}

	if (IsDamagePathBlocked(ActorToDamage))
	{
		return 0;
	}


	const float Distance = FVector::Distance(GetActorLocation(), ActorToDamage->GetActorLocation());

	if (Distance > MinDamage_Range)
	{
		UE_LOG(LogTemp, Warning, TEXT("PulseBomb tried to damage Actor outside MinDamage_Range."));
		return 0;
	}

	// Deal MaxDamage at all distances within MaxDamage_Range
	if (Distance <= MaxDamage_Range)
	{
		return MaxDamage;
	}

	// Deal MaxDamage at MaxDamage_Range distance, scaling linearly down to MinDamage if Distance == MinDamage_Range.
	//     The difference between MinDamage_Range and MaxDamage_Range determines how sharp the damage falloff is.
	// Note: The case where (MinDamage_Range - MaxDamage_Range == 0) will have already been covered by the above checks.
	const int Damage = FMath::Lerp(MaxDamage, MinDamage, ( (Distance - MaxDamage_Range) / (MinDamage_Range - MaxDamage_Range) ));

	UE_LOG(LogTemp, Log, TEXT("Distance to bomb: %f"), Distance);
	UE_LOG(LogTemp, Log, TEXT("Damage from bomb: %i"), Damage);

	return Damage;
}

bool ATPulseBomb::IsDamagePathBlocked(AActor* const ActorToDamage) const
{
	if (!ActorToDamage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pulse Bomb tried to check IsDamagePathBlocked for nullptr ActorToDamage. Returning true."));
		return true;
	}

	FCollisionObjectQueryParams Params;
	Params.AddObjectTypesToQuery(ECC_WorldDynamic); 
	Params.AddObjectTypesToQuery(ECC_WorldStatic);

	const FVector StartLocation = GetActorLocation();
	const FVector EndLocation = ActorToDamage->GetActorLocation();

	FHitResult HitResult;

	return GetWorld()->LineTraceSingleByObjectType(HitResult, StartLocation, EndLocation, Params);
}

void ATPulseBomb::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ensure instigator can't be stuck by their own bomb
	if (OtherActor == GetInstigator())
	{
		return;
	}

	// Only stick to first actor it overlaps with
	CollisionSphereComp->SetGenerateOverlapEvents(false);

	ProjectileMovementComp->SetVelocityInLocalSpace(FVector::Zero());
	ProjectileMovementComp->ProjectileGravityScale = 0.f;

	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pulse bomb overlapped with null OtherActor"));
		return;
	}

	FColor DebugColor = FColor::Blue;

	// If the other actor was a pawn, attach the bomb to them
	APawn* const OtherPawn = Cast<APawn>(OtherActor);
	if (OtherPawn)
	{
		MeshComp->SetVisibility(false);

		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
		AttachToActor(OtherPawn, TransformRules);

		DebugColor = FColor::Yellow;
	}	

	if (CVarPulseBombDebugLines.GetValueOnGameThread())
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 5.0f, 16, DebugColor, false, 3.0f, 2, 1.0f);
	}
}

void ATPulseBomb::ToggleLight()
{
	LightComp->SetVisibility(!LightComp->IsVisible());
}