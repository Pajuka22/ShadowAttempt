// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyUserWidget.h"
#include "SuperMenu.h"
#include "SubMenu.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWATTEMPT_API USubMenu : public UMyUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Behavior")
		USuperMenu* parent = NULL;
	virtual void Back() override;
private:

	
};
