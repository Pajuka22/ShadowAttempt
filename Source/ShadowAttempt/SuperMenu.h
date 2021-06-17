// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyUserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "SuperMenu.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWATTEMPT_API USuperMenu : public UMyUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "Behavior")
		UWidgetSwitcher* switcher;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
		USuperMenu* parent = NULL;
	virtual void Back() override;
	UFUNCTION(BlueprintCallable)
		virtual void SetSwitcherIndex(int index);
};
