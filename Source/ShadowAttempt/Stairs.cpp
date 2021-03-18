// Fill out your copyright notice in the Description page of Project Settings.
#include "Stairs.h"
#include "Components/StaticMeshComponent.h"
// Sets default values
AStairs::AStairs()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mesh"));

}

// Called when the game starts or when spawned
void AStairs::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStairs::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

