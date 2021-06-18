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
	UFUNCTION(BlueprintCallable)
		static TArray<FName> GetGameplayActionAxes();
	//UPROPERTY(BlueprintReadOnly, Category = "Player Controls|InputModes")
		static const TArray<FName> GameplayActionAxes;
	//UPROPERTY(BlueprintReadOnly, Category = "Player Controls|InputModes")
		static const TArray<FName> Unbindables;
};
