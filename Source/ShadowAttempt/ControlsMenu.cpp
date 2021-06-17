// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlsMenu.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h"

void UControlsMenu::SwapIfNeeded(FKeySelectorInfo binding, TArray<FKeySelectorInfo> matches) {
	UMyInputKeySelector *BB1 = GetBindButtonByInfo(binding);
	UMyInputKeySelector* BB2;
	if (BB1) {
		FInputChord toBind = BB1->SelectedKey;
		FInputChord beforeBinding = BB1->UpdateKeys();
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, toBind.GetInputText().ToString());
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, beforeBinding.GetInputText().ToString());
		UMyGameInstance* game = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this));
		for (int i = 0; i < matches.Num(); ++i) {
			//check if it's unbindable and no duplicates are allowed.
			if (game->Unbindables.Contains(matches[i].name) && DisallowDuplicates.Contains(matches[i].name)) {
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, "Tried swapping with unbindable. can't.");
				return;
			}
			BB2 = GetBindButtonByInfo(matches[i]);
			if (BB2) {
				//check if BB2 can bind the input. if it can't return.
				if (!BB2->CanBindInput(beforeBinding)) {
					GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, "Could not find a bind button.");
					return;
				}
			}
			//check if the button is allowed to have duplicates if there's no bind button for it.
			else if (DisallowDuplicates.Contains(matches[i].name)) {
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, "Button is bindable but can't be changed back. aborting.");
				BB1->DontValidateOnce();
				BB1->SetSelectedKey(beforeBinding);
				return;
			}
		}
		//this is specifically for instances where we're swapping buttons on the same axis.
		BB1->DontValidateOnce();
		BB1->Unbind();
		for (int i = 0; i < matches.Num(); ++i) {
			BB2 = GetBindButtonByInfo(matches[i]);
			if (BB2) {
				BB2->DontValidateOnce();
				BB2->SetSelectedKey(beforeBinding);
			}
		}
		BB1->Bind(toBind);
	}
}

UMyInputKeySelector* UControlsMenu::GetBindButtonByInfo(FKeySelectorInfo findBy) {
	for (int i = 0; i < bindButtons.Num(); ++i) {
		if (bindButtons[i]->info.name == findBy.name && findBy.isAction == bindButtons[i]->info.isAction &&
			(findBy.isAction ? true : FMath::Sign(findBy.axisScale) == FMath::Sign(bindButtons[i]->info.axisScale))) {
			return bindButtons[i];
		}
	}
	return NULL;
}

TArray<UMyInputKeySelector*>& UControlsMenu::GetBindButtons() {
	return bindButtons;
}

void UControlsMenu::AddElementToBindButtons(UMyInputKeySelector* toAdd) {
	bindButtons.Add(toAdd);
}

void UControlsMenu::PrepBindButtons() {
	for (int i = 0; i < bindButtons.Num(); ++i) {
		bindButtons[i]->Testing.AddDynamic(this, &UControlsMenu::SwapIfNeeded);
		bindButtons[i]->UpdateKeys();
	}
}