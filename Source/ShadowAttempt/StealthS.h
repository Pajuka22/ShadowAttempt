// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerPawn.h"
#include "PlayerVisibility.h"
#include "StealthS.generated.h"

UCLASS()
class SHADOWATTEMPT_API AStealthS : public AActor
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		class USpotLightComponent* Source;

public:
	// Sets default values for this actor's properties
	AStealthS();

	class APlayerPawn* Player;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool HitLast = false;
	PlayerVisibility value;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};