// Fill out your copyright notice in the Description page of Project Settings.


#include "SneakOverride.h"

// Sets default values for this component's properties
USneakOverride::USneakOverride()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}


// Called when the game starts
void USneakOverride::BeginPlay()
{
	Super::BeginPlay();
	if (Normal == FVector::ZeroVector) Normal = GetUpVector();
	else if (Relative) Normal = GetForwardVector() * Normal.X + GetRightVector() * Normal.Y + GetUpVector() * Normal.Z;
	Normal.Normalize();
	tolerance *= PI / 180;
	// ...
	
}


// Called every frame
/*void USneakOverride::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}*/
