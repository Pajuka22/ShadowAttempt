// Fill out your copyright notice in the Description page of Project Settings.


#include "MyInputKeySelector.h"
#include "GameFramework/InputSettings.h"
#include "Components/InputKeySelector.h"
#include "Framework/Commands/InputChord.h"
#include "GameFramework/PlayerInput.h"
#include "KeySelectorInfo.h"
#include "GameFramework/PlayerInput.h"

UMyInputKeySelector::UMyInputKeySelector() {
	FScriptDelegate validate;
	validate.BindUFunction(this, "ValidateInput");
	OnKeySelected.Add(validate);
}

FInputChord& UMyInputKeySelector::UpdateKeys()
{
	bAllowModifierKeys = !info.isAction;
	bool foundIt = false;
	if (info.isAction) {
		TArray<FInputActionKeyMapping> mappings;
		UInputSettings::GetInputSettings()->GetActionMappingByName(info.name, mappings);
		for (int i = 0; i < mappings.Num(); ++i) {
			if (mappings[i].Key.IsGamepadKey() == bAllowGamepadKeys) {
				foundIt = true;
				SelectedKey.Key = mappings[i].Key;
				SelectedKey.bAlt = mappings[i].bAlt;
				SelectedKey.bCmd = mappings[i].bCmd;
				SelectedKey.bCtrl = mappings[i].bCtrl;
				SelectedKey.bShift = mappings[i].bShift;
				//SetSelectedKey(SelectedKey);
				SetNoKeySpecifiedText(SelectedKey.GetInputText());
				break;
			}
		}
	}
	else {
		TArray<FInputAxisKeyMapping> mappings;
		UInputSettings::GetInputSettings()->GetAxisMappingByName(info.name, mappings);
		for (int i = 0; i < mappings.Num(); ++i) {
			if (mappings[i].Key.IsGamepadKey() == bAllowGamepadKeys && FMath::Sign(mappings[i].Scale) == FMath::Sign(info.axisScale)) {
				foundIt = true;
				SelectedKey = FInputChord(mappings[i].Key, false, false, false, false);
				SetNoKeySpecifiedText(SelectedKey.GetInputText());
				break;
			}
		}
	}
	if (!foundIt) {
		SetNoKeySpecifiedText(FText::FromString(TEXT("Empty")));
	}
	return SelectedKey;
}

void UMyInputKeySelector::Bind(const FInputChord input)
{
	Unbind();
	if (info.isAction) BindAction(input);
	else BindAxis(input);
}

void UMyInputKeySelector::Unbind() {
	if (info.isAction) {
		TArray<FInputActionKeyMapping> mappings;
		UInputSettings::GetInputSettings()->GetActionMappingByName(info.name, mappings);
		for (int i = 0; i < mappings.Num(); ++i) {
			if (mappings[i].Key.IsGamepadKey() == bAllowGamepadKeys) {
				UInputSettings::GetInputSettings()->RemoveActionMapping(mappings[i]);
			}
		}
	}
	else {
		TArray<FInputAxisKeyMapping> mappings;
		UInputSettings::GetInputSettings()->GetAxisMappingByName(info.name, mappings);
		for (int i = 0; i < mappings.Num(); ++i) {
			if (mappings[i].Key.IsGamepadKey() == bAllowGamepadKeys && FMath::Sign(mappings[i].Scale) == FMath::Sign(info.axisScale)) {
				UInputSettings::GetInputSettings()->RemoveAxisMapping(mappings[i]);
			}
		}
	}
}

void UMyInputKeySelector::BindAction(FInputChord input) {
	FInputActionKeyMapping toMap(info.name, input.Key, input.bShift, input.bCtrl, input.bAlt, input.bCmd);
	UInputSettings::GetInputSettings()->AddActionMapping(toMap, true);
	UpdateKeys();
}

void UMyInputKeySelector::BindAxis(FInputChord input) {

	FInputAxisKeyMapping toMap(info.name, input.Key, info.axisScale);
	
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Magenta, FString(TEXT("Binding ")).Append(input.GetInputText().ToString()).Append(" to ").Append(ToString()));
	UInputSettings::GetInputSettings()->AddAxisMapping(toMap, true);
	UpdateKeys();
}

void UMyInputKeySelector::ValidateInput(FInputChord input) {
	if (!CanBindInput(input)) {
		UpdateKeys();
		return;
	}
	TArray<FKeySelectorInfo> matches;
	UInputSettings* s = UInputSettings::GetInputSettings();
	TArray<FInputActionKeyMapping> actions = s->GetActionMappings();
	FKeySelectorInfo x;
	for (int i = 0; i < actions.Num(); ++i) {
		x = SharesInputWithMapping(input, actions[i]);
		if(x.name != "None") matches.Add(x);
	}
	TArray<FInputAxisKeyMapping> axes = s->GetAxisMappings();
	for (int i = 0; i < axes.Num(); ++i) {
		x = SharesInputWithMapping(input, axes[i]);
		if (x.name != "None") matches.Add(x);
	}
	if (matches.Num() == 0 || !callValidate) {
		Bind(input);
	}
	else {
		Testing.Broadcast(info, matches);
		callValidate = false;
		UpdateKeys();
	}
	callValidate = true;
}

FKeySelectorInfo UMyInputKeySelector::SharesInputWithMapping(const FInputChord& chord, const FInputActionKeyMapping& mapping) {
	FKeySelectorInfo match;
	match.name = "None";
	match.isAction = true;
	FInputChord mappingChord(mapping.Key, mapping.bShift, mapping.bCtrl, mapping.bAlt, mapping.bCmd);
	if (mapping.ActionName != info.name && mappingChord == chord) {
		match.name = mapping.ActionName;
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, mapping.ActionName.ToString());
	}
	
	return match;
}

FKeySelectorInfo UMyInputKeySelector::SharesInputWithMapping(const FInputChord& chord, const FInputAxisKeyMapping& mapping) {
	FKeySelectorInfo match;
	match.name = "None";
	match.isAction = false;
	FInputChord mappingChord(mapping.Key, false, false, false, false);
	if (((mapping.AxisName == info.name && FMath::Sign(mapping.Scale) != FMath::Sign(info.axisScale)) || mapping.AxisName != info.name) && mappingChord == chord) {
		match.name = mapping.AxisName;
		match.axisScale = mapping.Scale;
	}
	return match;
}

void UMyInputKeySelector::DontValidateOnce() {
	callValidate = false;
}

bool UMyInputKeySelector::CanBindInput(const FInputChord input) {
	bool b = info.isAction || !(input.bAlt || input.bCmd || input.bCtrl || input.bShift);
	if (!b) GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, "Couldn't bind input");
	return b;
}

FString UMyInputKeySelector::ToString() {
	FString s;
	if (!info.isAction && info.axisScale < 0) {
		s.Append("Negative ");
	}
	s.Append(info.name.ToString());
	return s;
}