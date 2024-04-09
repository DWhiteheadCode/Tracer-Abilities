// Fill out your copyright notice in the Description page of Project Settings.


#include "TPulseBomb.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ATPulseBomb::ATPulseBomb()
{
	StickRadius = 20;
	ExplosionDelay = 2;
	ExplosionRadius = 200;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionProfileName("NoCollision");
	RootComponent = MeshComp;

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComp");
	ProjectileMovementComp->InitialSpeed = 1250;
	ProjectileMovementComp->ProjectileGravityScale = 3;

	CollisionSphereComp = CreateDefaultSubobject<USphereComponent>("CollisionSphereComp");
	CollisionSphereComp->SetupAttachment(RootComponent);
	CollisionSphereComp->SetSphereRadius(StickRadius);
	CollisionSphereComp->SetCollisionProfileName("Projectile");
	CollisionSphereComp->SetGenerateOverlapEvents(true);
}

void ATPulseBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CollisionSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ATPulseBomb::OnBeginOverlap);
}

void ATPulseBomb::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Explosion, this, &ATPulseBomb::Explode, ExplosionDelay, false);
}


void ATPulseBomb::Explode()
{
	UE_LOG(LogTemp, Log, TEXT("Pulse bomb exploded"));

	MeshComp->SetVisibility(false);
	SetActorEnableCollision(false);
	SetLifeSpan(2.f);
}

void ATPulseBomb::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != GetInstigator())
	{
		UE_LOG(LogTemp, Log, TEXT("PULSE BOMB OVERLAPPED"));

		// Should only stick to first actor it overlaps with
		CollisionSphereComp->SetGenerateOverlapEvents(false);
	}
}
