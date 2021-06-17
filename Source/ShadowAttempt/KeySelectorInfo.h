// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KeySelectorInfo.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FKeySelectorInfo {
	GENERATED_BODY();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Variables")
	FName name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Variables")
	bool isAction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Variables")
	float axisScale;

};