// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthS.h"
#include "Runtime/Engine/Classes/Components/SpotLightComponent.h"
#include "Runtime/Engine/Classes/Engine/Scene.h"
#include "Engine/World.h"
#include "PlayerPawn.h"
#include "PlayerVisibility.h"
// Sets default values
AStealthS::AStealthS()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Source = CreateDefaultSubobject<USpotLightComponent>(TEXT("Source"));
	Source->IntensityUnits = ELightUnits::Candelas;
	Source->Intensity = 500;
}

// Called when the game starts or when spawned
void AStealthS::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<APlayerPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

// Called every frame
void AStealthS::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HitLast) {
		Player->MyVis -= value;
	}
	value = Player->SStealth(GetActorLocation(), Source->InnerConeAngle, Source->OuterConeAngle, Source->AttenuationRadius, GetActorForwardVector(), Source->Intensity);
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, FString::SanitizeFloat(value.Vis));
	HitLast = value.Vis >= 0;
	Player->MyVis += value;
}

