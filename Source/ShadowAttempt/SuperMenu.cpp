// Fill out your copyright notice in the Description page of Project Settings.


#include "SuperMenu.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerPawn.h"

void USuperMenu::Back() {
	UMyUserWidget* active = Cast<UMyUserWidget>(switcher->GetActiveWidget());
	if (active) {
		APlayerPawn* player = Cast<APlayerPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (active->prev < 0) {
			if (parent) {
				parent->Back();
			}
			else {
				if (player) player->Pause();
				else {
					UGameplayStatics::SetGamePaused(this, false);
					RemoveFromParent();
				}
			}
		}
		else {
			if (parent) parent->Back();
			else if (player) player->Pause();
			else {
				UGameplayStatics::SetGamePaused(this, false);
				RemoveFromParent();
			}
		}
	}
}

void USuperMenu::SetSwitcherIndex(int index) {
	if (index < switcher->GetNumWidgets() && index > 0) {
		switcher->SetActiveWidgetIndex(index);
	}
	else {
		Back();
	}
}