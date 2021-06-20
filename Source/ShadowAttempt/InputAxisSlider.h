// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "InputAxisSlider.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWATTEMPT_API UInputAxisSlider : public USlider
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		TArray<FName> AffectedAxes;
	UPROPERTY(EditAnywhere)
		bool bGamepad = false;

public:
	UInputAxisSlider();
	UFUNCTION(BlueprintCallable)
		void PrepSlider();
	UFUNCTION(BlueprintCallable)
		void SetAxesValues(float val);
};
