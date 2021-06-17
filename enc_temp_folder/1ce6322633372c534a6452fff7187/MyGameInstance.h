// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWATTEMPT_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Controls|Camera")
	float LookScaleX = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Controls|Camera")
	float LookScaleY = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Player Controls|InputModes")
		TArray<FName> GamplayActionAxes = 
	{ 
		TEXT("MoveForward"), 
		TEXT("MoveRight"), 
		TEXT("LookUp"), 
		TEXT("LookUpAtRate"), 
		TEXT("Turn"), 
		TEXT("TurnAtRate"),
		TEXT("Jump"),
		TEXT("Sprint"),
		TEXT("Crouch"),
		TEXT("Sneaky"),
	};

	UPROPERTY(BlueprintReadOnly, Category = "Player Controls|InputModes")
	TArray<FName> Unbindables =
	{
		TEXT("LookUp"),
		TEXT("LookUpAtRate"),
		TEXT("LookRight"),
		TEXT("LookRightAtRate")
	};
};
