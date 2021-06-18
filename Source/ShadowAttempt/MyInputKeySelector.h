// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InputKeySelector.h"
#include "Framework/Commands/InputChord.h"
#include "GameFramework/PlayerInput.h"
#include "KeySelectorInfo.h"
#include "MyInputKeySelector.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTestDelegate, FKeySelectorInfo, binding, TArray<FKeySelectorInfo>, matches);

UCLASS()
class SHADOWATTEMPT_API UMyInputKeySelector : public UInputKeySelector
{
	GENERATED_BODY()
public:

	UMyInputKeySelector();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
		FKeySelectorInfo info;

	void Bind(const FInputChord input);
	UFUNCTION(BlueprintCallable)
		FInputChord& UpdateKeys();

	bool CanBindInput(const FInputChord input);

	UPROPERTY(BlueprintAssignable, Category = "Test")
		FTestDelegate Testing;

	void DontValidateOnce();
	FString ToString();
	void Unbind();
private:
	bool bindable;
	void BindAction(const FInputChord input);
	void BindAxis(const FInputChord input);
	UFUNCTION()
		void ValidateInput(const FInputChord input);
	bool callValidate = true;
	FKeySelectorInfo SharesInputWithMapping(const FInputChord &chord, const FInputAxisKeyMapping &mapping);
	FKeySelectorInfo SharesInputWithMapping(const FInputChord &chord, const FInputActionKeyMapping &mapping);
};