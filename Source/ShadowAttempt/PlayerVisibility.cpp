// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerVisibility.h"

PlayerVisibility::PlayerVisibility()
{
	Vis = 0; 
	GroundVis = 0;
}
PlayerVisibility::PlayerVisibility(float v, float g) 
{
	Vis = v;
	GroundVis = g;
}

PlayerVisibility::~PlayerVisibility()
{
}

FString PlayerVisibility::ToString() {
	FString string = "Total: " + FString::SanitizeFloat(Vis) + "\nGround: " + FString::SanitizeFloat(GroundVis);
	return string;
}

void PlayerVisibility::Set(float v, float g) {
	Vis = v;
	GroundVis = g;
}