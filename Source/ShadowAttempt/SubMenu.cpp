// Fill out your copyright notice in the Description page of Project Settings.


#include "SubMenu.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerPawn.h"

void USubMenu::Back() {
	if (parent) {
		parent->Back();
	}
	else {
		APlayerPawn* player = Cast<APlayerPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (player) player->Pause();
		else {
			UGameplayStatics::SetGamePaused(this, false);
			RemoveFromParent();
		}
	}
}