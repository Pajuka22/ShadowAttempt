// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CustomMovement.generated.h"

/**
 *
 */
UCLASS()
class SHADOWATTEMPT_API UCustomMovement : public UPawnMovementComponent
{
	GENERATED_BODY()


public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere)
		float StepHeight = 30;
	/*Max slope player can walk up/down.*/
	UPROPERTY(EditAnywhere)
		float MaxAngle = 50;
	/*Max angle the player can wall walk on relative to current up vector*/
	UPROPERTY(EditAnywhere)
		float SneakMaxAngle = 90;
	bool Running;
	FVector DownVel = FVector::ZeroVector;
	FVector JumpVel = FVector::ZeroVector;
	FVector LateralVel = FVector::ZeroVector;
	FVector HeightAdjustVel = FVector::ZeroVector;
	FVector CurrentLatVel;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float SneakSpeed = 600;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float CrouchSpeed = 300;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float NormalSpeed = 400;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float SprintSpeed = 750;
	float MovementSpeed;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"));
		float JumpHeight = 2;
	UPROPERTY(EditAnywhere)
		float Gravity = 9.81;
	bool Shadow;
	float JumpSpeed;
	enum MovementType{Walk, Sprint, Crouch, Sneak};
	MovementType MoveType;
	void SetSpeed();
	void Jump();
	bool CheckGrounded();
	bool CanJump();
	bool Stepping = false;
	bool StartJump = false;
	bool EndJump = false;
	class APlayerPawn* Pawn;
	int GroundNum = 0;
	UCapsuleComponent* Capsule;
	bool Walking;
	bool bGroundedCache;
	bool CheckStepUp(FVector movement);

protected:
	virtual void BeginPlay() override;

	bool CheckStepDown(FVector movement);
	float GetStepHeight(FVector movement);
	bool CheckEndJump();
	
	FVector SlopeDownVel;
};