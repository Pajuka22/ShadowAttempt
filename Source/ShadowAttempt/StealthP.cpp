// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthP.h"
#include "Components/PointLightComponent.h"
#include "Runtime/Engine/Classes/Engine/Scene.h"
#include "Engine/World.h"
#include "PlayerPawn.h"

// Sets default values
AStealthP::AStealthP()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Source = CreateDefaultSubobject<UPointLightComponent>(TEXT("Source"));
	Source->IntensityUnits = ELightUnits::Candelas;
	Source->Intensity = 100;
}

// Called when the game starts or when spawned
void AStealthP::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<APlayerPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

// Called every frame
void AStealthP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HitLast) {
		Player->MyVis -= value;
	}
	value = Player->PStealth(GetActorLocation(), Source->AttenuationRadius, Source->Intensity);
	HitLast = value.Vis >= 0;
	Player->MyVis += value;
}

