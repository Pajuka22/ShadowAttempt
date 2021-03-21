// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SHADOWATTEMPT_API PlayerVisibility
{
public: 

	float Vis;
	float GroundVis;

public:

	PlayerVisibility();
	PlayerVisibility(float v, float g);
	~PlayerVisibility();
	
public:

	FORCEINLINE PlayerVisibility operator+(const PlayerVisibility& V) const {
		return PlayerVisibility(Vis + V.Vis, GroundVis + V.GroundVis);
	}
	FORCEINLINE PlayerVisibility operator+=(const PlayerVisibility& V) {
		Vis += V.Vis;
		GroundVis += V.GroundVis;
		return *this;
	}
	FORCEINLINE PlayerVisibility operator-(const PlayerVisibility& V) const {
		return PlayerVisibility(Vis - V.Vis, GroundVis - V.GroundVis);
	}
	FORCEINLINE PlayerVisibility operator-=(const PlayerVisibility& V) {
		Vis -= V.Vis;
		GroundVis -= V.GroundVis;
		return *this;
	}

public:
	FString ToString();
	void Set(float v, float f);
};
