// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SneakCamOverride.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHADOWATTEMPT_API USneakCamOverride : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USneakCamOverride();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
		float Influence;
};
