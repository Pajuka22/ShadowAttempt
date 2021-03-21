// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerPawn.h"
#include "PlayerVisibility.h"
#include "StealthD.generated.h"
UCLASS()
class SHADOWATTEMPT_API AStealthD : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStealthD();

	class APlayerPawn* Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDirectionalLightComponent* Source;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool HitLast = false;
	PlayerVisibility value;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
