// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CustomMovement.generated.h"
#include "Curves/CurveFloat.h"

/**
 *
 */
UCLASS()
class SHADOWATTEMPT_API UCustomMovement : public UPawnMovementComponent
{
	GENERATED_BODY()


public:
	UCustomMovement();
	enum MoveTypes{
		Walk,
		Crouch,
		Sprint,
		Sneak
	}
	


private:
	UPROPERTY(EditAnywhere, Category = "Size")
	float normalRadius;
	
	UPROPERTY(EditAnywhere, Category = "Size")
	float normalHeight;
	
	UPROPERTY(EditAnywhere, Category = "Size")
	float sneakHeight;
	
	UPROPERTY(EditAnywhere, Category = "Size")
	float crouchHeight;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Traversal", meta = (ClampMin = "0", ClampMax =  "90"))
	float maxAngle;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Traversal", meta = (ClampMin = "0"))
	float stepHeight;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Speed", meta = (ClampMin = "0"))
	float walkSpeed = 400;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Speed", meta = (ClampMin = "0"))
	float sprintSpeed = 750;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Speed", meta = (ClampMin = "0"))
	float crouchSpeed = 300;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Speed", meta = (ClampMin = "0"))
	float sneakSpeed = 600;
	
	UPROPERTY(EditAnywhere, Category = "Shadow Sneak", meta = (ClampMin = "0"))
	float stopSneakTime;
	
	UPROPERTY(EditAnywhere, Category = "Shadow Sneak", meta = (ClampMin = "0"))
	float sneakBufferWindow;
	
	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
	//used to determine rotate speed depending on what the initial difference in the axes was.
	UCurveFloat* rotateSpeedByInitialDifference;
	
	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
	//used to determine rotate speed depending on what the actual difference was.
	UCurveFloat* rotateSpeedByActualDifference;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak", meta = (ClampMin = "0", ClampMax = "1"))
	float camSneakInfluence;
	
	class UCameraComponent* playerCam = NULL;
	
	float notGroundedTime;
	float heightInterpTime;
	
	MoveTypes buffer;
	MoveTypes moveType;
	float moveSpeed;
	bool startJump;
	bool endJump;
	float angleAtStartRotate;

	FVector downVel;
	FVector jumpVel;
	FVector lateralVel;
	FVector heightAdjustVel;

	bool shadowSneak;
	FVector floorNormal = FVector::UpVector;
	FVector desiredUp = FVector::UpVector;
	int groundNum;
	float distInMoveDir;

	UCapsuleComponent* capsule;

public:
	void SetMoveType(MoveTypes);
	void SetMoveSpeed();
	void Jump();
	void SetCamSneakInfluence(float);
	void SneakControl();
	void StartSneak();
	void EndSneak();
	void Sprint();
	void StopSprinting();
	void Crouch();
	void StopCrouching();

private:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)

	void RotatePlayer(float DeltaTime);

	void CheckStepUp();
	void CheckStepDown();

	bool CanSneak();
	bool CanCrouch();
	bool CanSprintNormal();
	float GetHeightAdjustment();
	void AdjustHeight();

	bool HittingBottom(FVector hitPos, float maxDeg = 90, bool top = false);
	bool HittingSides(FVector hitpos);
};