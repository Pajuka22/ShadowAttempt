// Fill out your copyright notice in the Description page of Project Settings.
#include "StealthD.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"
#include "Runtime/Engine/Classes/Engine/Scene.h"
#include "Engine/World.h"
#include "PlayerPawn.h"
#include "PlayerVisibility.h"

// Sets default values
AStealthD::AStealthD()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Source = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Source"));
	Source->Intensity = 3;
}

// Called when the game starts or when spawned
void AStealthD::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<APlayerPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

// Called every frame
void AStealthD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Player != nullptr) {
		if (HitLast) {
			Player->MyVis -= value;
		}
		value = Player->DStealth(GetActorForwardVector(), Source->Intensity, 5000);
		HitLast = value.Vis >= 0;
		Player->MyVis += value;
	}
}

