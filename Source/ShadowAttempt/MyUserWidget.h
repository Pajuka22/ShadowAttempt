// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWATTEMPT_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Behavior")
		virtual void Back();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
		int prev = -1;
};