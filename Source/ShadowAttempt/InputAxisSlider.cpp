// Fill out your copyright notice in the Description page of Project Settings.


#include "InputAxisSlider.h"
#include "GameFramework/InputSettings.h"

UInputAxisSlider::UInputAxisSlider() {
	FScriptDelegate del;
	del.BindUFunction(this, "SetAxesValues");
	OnValueChanged.Add(del);
}

void UInputAxisSlider::SetAxesValues(float val) {
	for (int i = 0; i < AffectedAxes.Num(); ++i) {
		TArray<FInputAxisKeyMapping> bindings;
		UInputSettings::GetInputSettings()->GetAxisMappingByName(AffectedAxes[i], bindings);
		for (int j = 0; j < bindings.Num(); ++j) {
			if (bindings[j].Key.IsGamepadKey() == bGamepad) {
				FInputAxisKeyMapping toMap(bindings[j]);
				toMap.Scale = FMath::Sign(toMap.Scale) * val;
				UInputSettings::GetInputSettings()->RemoveAxisMapping(bindings[j]);
				UInputSettings::GetInputSettings()->AddAxisMapping(toMap, true);
			}
		}
	}
	UInputSettings::GetInputSettings()->SaveKeyMappings();
}

void UInputAxisSlider::PrepSlider() {
	TArray<FInputAxisKeyMapping> bindings;
	//this is only in a loop to make sure we get something valid.
	for (int i = 0; i < AffectedAxes.Num(); ++i) {
		UInputSettings::GetInputSettings()->GetAxisMappingByName(AffectedAxes[i], bindings);
		for (int j = 0; j < bindings.Num(); ++j) {
			SetValue(FMath::Abs(bindings[j].Scale));
			return;
		}
	}
}