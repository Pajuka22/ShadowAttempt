// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubMenu.h"
#include "MyInputKeySelector.h"
#include "KeySelectorInfo.h"
#include "ControlsMenu.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWATTEMPT_API UControlsMenu : public USubMenu
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		TArray<UMyInputKeySelector*>& GetBindButtons();
	UFUNCTION(BlueprintCallable)
		void AddElementToBindButtons(UMyInputKeySelector* toAdd);

private:
	UPROPERTY(EditAnywhere, Category = "Binding")
		TArray<UMyInputKeySelector*> bindButtons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pickup, meta = (AllowPrivateAccess = "true"))
		TArray<FName> DisallowDuplicates;

	UMyInputKeySelector* GetBindButtonByInfo(FKeySelectorInfo findBy);
	UFUNCTION(BlueprintCallable)
		void SwapIfNeeded(FKeySelectorInfo binding, TArray<FKeySelectorInfo> matches);
	UFUNCTION(BlueprintCallable)
		void PrepBindButtons();
};
