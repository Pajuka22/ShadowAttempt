// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

const TArray<FName> UMyGameInstance::Unbindables = {
		TEXT("LookUp"),
		TEXT("LookUpAtRate"),
		TEXT("LookRight"),
		TEXT("LookRightAtRate"),
		TEXT("Pause")
};

const TArray<FName> UMyGameInstance::GameplayActionAxes =
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

TArray<FName> UMyGameInstance::GetGameplayActionAxes() {
	return GameplayActionAxes;
}