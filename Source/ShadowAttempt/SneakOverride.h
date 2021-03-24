// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SneakOverride.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHADOWATTEMPT_API USneakOverride : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USneakOverride();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	/*FVector to set desired up to. defaults to actor's up vector*/
	UPROPERTY(EditAnywhere)
		FVector Normal = FVector::ZeroVector;
	/*is it relative?*/
	UPROPERTY(EditAnywhere)
		bool relative;
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
