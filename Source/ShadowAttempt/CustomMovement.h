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
		float SneakSpeed = 200;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float CrouchSpeed = 300;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float NormalSpeed = 300;
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
		float SprintSpeed = 600;
	float MovementSpeed;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"));
		float JumpHeight;
	UPROPERTY(EditAnywhere)
		float Gravity = 9.81;
	bool Shadow;
	float JumpSpeed = 800;
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