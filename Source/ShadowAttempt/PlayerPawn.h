// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

UCLASS()
class SHADOWATTEMPT_API APlayerPawn : public APawn
{
	GENERATED_BODY()
public:
	// Sets default values for this pawn's properties
	APlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* VisibleComponent;
	UPROPERTY(EditAnywhere)
		class UCustomMovement* MovementComp;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	class UCapsuleComponent* Capsule;

	class UCameraComponent* MyCamera;


	UPROPERTY(EditAnywhere)
		float sneakHeight = 16;
	UPROPERTY(EditAnywhere)
		float crouchHeight = 50;
	UPROPERTY(EditAnywhere)
		float normalHeight = 100;
	UPROPERTY(EditAnywhere)
		float HeightInterpTime = 0.3;
	UPROPERTY(EditAnywhere)
		float SneakThreshold;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "100"))
		float NormalRadius = 50;
	UPROPERTY(EditAnywhere, Category = "Camera|Camera Motion|Shake")
		class UCurveVector* WalkCurve;
	UPROPERTY(EditAnywhere, Category = "Camera|Camera Motion|Shake")
		float WalkCurveStartLoop = 0;
	UPROPERTY(EditAnywhere, Category = "Camera|Camera Motion|Shake")
		float WalkCurveEndLoop = 3;

	UPROPERTY(EditAnywhere, Category = "Camera|Camera Motion|Shadow Sneak", meta = (ClampMin = "0", ClampMax = "1"))
		float CamLerpMult = 1;
	/*Adds some buffer to dropping off of a wall if not grounded in shadow mode*/
	UPROPERTY(EditAnywhere)
		int ShadowDropTime = 3;

	bool CheckGrounded();

	bool ShadowSneak;
	int Grounded;
	bool bCrouch;
	bool bSprint;
	bool bBufferSprint;
	bool bBufferEndSprint;
	struct Visibility {
		float Vis;
		float GroundVis;
	};

	Visibility  DStealth(FVector angle, float magnitude, float length);
	Visibility SStealth(FVector spotlight, float inner, float outer, float Attenuation, FVector spotAngle, float Candelas);
	Visibility PStealth(FVector position, float attenuation, float Candelas);

	Visibility MyVis;
	void AddVis(Visibility vis);
	void SubVis(Visibility vis);
	float GetCapsuleVisibleArea();

	FVector FloorNormal;
	FVector OldNormal;
	UPROPERTY(EditAnywhere)
		float MaxRotateSpeed;
	FVector Under;
	UPROPERTY(EditAnywhere)
		float MaxHP;

	float CurrentHP;

protected:
	float CurveTime = 0;
	float startHeight;
	float endHeight;
	float currentHeight;
	float addHeight;

	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void StartEndSneak();
	void Jump();
	void StopJumping();

	void Sprint();
	void StopSprinting();
	void CrouchControl();
	void Crouch();
	void StopCrouching();
	void BufferEndCrouch();
	void GetAddHeight();

	bool HittingBottom(FVector hitPos, float maxDeg = 90);
	bool IsStepUp(FVector hitPos);

	float FloorAngle = 60;
	int notGroundedTime;


	UFUNCTION()
		void RootCollision(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void RootCollisionExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void RootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

};