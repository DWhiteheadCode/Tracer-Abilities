// Fill out your copyright notice in the Description page of Project Settings.


#include "TPulseBomb.h"

#include "Components/StaticMeshComponent.h"

ATPulseBomb::ATPulseBomb()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	RootComponent = MeshComp;

	ExplosionDelay = 2;
	ExplosionRadius = 200;
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
