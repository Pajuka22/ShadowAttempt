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
		float StepHeight;
	UPROPERTY(EditAnywhere)
		float MaxAngle = 50;
	bool Running;
	FVector downVel = FVector::ZeroVector;
	FVector JumpVel = FVector::ZeroVector;
	FVector LateralVel;
	FVector CurrentLatVel;
	UPROPERTY(EditAnywhere)
		float SneakSpeed = 200;
	UPROPERTY(EditAnywhere)
		float CrouchSpeed = 300;
	UPROPERTY(EditAnywhere)
		float NormalSpeed = 300;
	UPROPERTY(EditAnywhere)
		float SprintSpeed = 600;
	float MovementSpeed;
	UPROPERTY(EditAnywhere)
		float JumpSpeed = 800;
	UPROPERTY(EditAnywhere)
		float Gravity = 9.81;
	bool Shadow;
	enum MovementType{Walk, Sprint, Crouch, Sneak};
	MovementType MoveType;
	void SetSpeed();
	void Jump();
	bool CheckGrounded();
	bool CanJump();
	bool Stepping;
	bool StartJump;
	bool EndJump;
	class APlayerPawn* Pawn;
	int GroundNum = 0;
	UCapsuleComponent* Capsule;
	bool Walking;
	bool bGroundedCache;

protected:
	virtual void BeginPlay() override;
	bool CheckStepUp(FVector movement);
	bool CheckStepDown(FVector movement);
	bool CheckEndJump();
};