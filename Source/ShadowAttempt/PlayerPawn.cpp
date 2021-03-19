// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Components/InputComponent.h"
#include "Runtime/HeadMountedDisplay/Public/MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "CustomMovement.h"
#include "Runtime/Engine/Classes/GameFramework/FloatingPawnMovement.h"
//#include "ConstructorHelpers.h"
#include "Runtime/Core/Public/Math/Vector.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Core/Public/Math/TransformNonVectorized.h"
#include "Runtime/Engine/Classes/Curves/CurveVector.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Stairs.h"
#include "SneakIgnore.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root Comp"));

	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->SetCapsuleSize(NormalRadius, NormalHeight);
	//Capsule->OnComponentBeginOverlap.AddDynamic(this, &APlayerPawn::RootCollision);
	//Capsule->OnComponentEndOverlap.AddDynamic(this, &APlayerPawn::RootCollisionExit);
	//Capsule->OnComponentHit.AddDynamic(this, &APlayerPawn::RootHit);
	RootComponent = Capsule;

	MyCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	MyCamera->SetupAttachment(RootComponent);
	MyCamera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
	MyCamera->SetRelativeRotation(FRotator(0, 0, 0));

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	MovementComp = CreateDefaultSubobject<UCustomMovement>(TEXT("Movement"));
	MovementComp->UpdatedComponent = RootComponent;
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovementComp->GroundNum = Grounded;
	//if it's not grounded
	if (!CheckGrounded() && !MovementComp->bGroundedCache) {
		//if more than the allowed number of frames have passed since it stopped being grounded
		if (notGroundedTime >= ShadowDropTime) {
			//return to default up direction
			FloorNormal = FVector::UpVector;
			if (ShadowSneak) StartEndSneak();
		}
		else ++notGroundedTime;
	}
	else {
		//reset timer if it's grounded
		notGroundedTime = 0;
	}
	//reset grounded and floor angle
	Grounded = 0;
	FloorAngle = PI / 2;

	if (bBufferSprint) {
		Sprint();
	}

	//rotating the player
	FVector tmpDesiredUp = ShadowSneak ? DesiredUp : FVector::UpVector;
	if (!FVector::Coincident(tmpDesiredUp, GetActorUpVector())) {

		//store cam forward before rotating.
		FVector camForward = MyCamera->GetForwardVector();

		//CP stands for cross product here.
		FVector CP = FVector::CrossProduct(GetActorUpVector(), tmpDesiredUp);
		CP.Normalize();

		//find new up vector, avoiding overrotation.
		FVector newUp = GetActorUpVector().RotateAngleAxis(FMath::Clamp(MaxRotateSpeed * DeltaTime, 0.0f, GetActorUpVector().RadiansToVector(tmpDesiredUp) * 180 / PI), CP);
		//FVector newUp = GetActorUpVector().RotateAngleAxis(MaxRotateSpeed * DeltaTime, CP);
		FVector newForward = FVector::CrossProduct(RootComponent->GetRightVector(), newUp);
		FVector newRight = FVector::CrossProduct(newUp, newForward);
		//construct a quat to rotate player to.
		FQuat q = FTransform(newForward, newRight, newUp, GetActorLocation()).GetRotation();
		//store actual angle rotated to reference when I rotate the camera.
		float angle = q.AngularDistance(GetActorQuat());
		RootComponent->SetWorldRotation(q);

		//desired look direction

		//up direction. keep in mind that looking side to side rotates the entire player, so cam forward and actor right will always be perpendicular.
		if (!FVector::Coincident(MyCamera->GetForwardVector(), camForward)) {
			newUp = FVector::CrossProduct(camForward, GetActorRightVector());
			q = FTransform(camForward, GetActorRightVector(), newUp, MyCamera->GetComponentLocation()).GetRotation();
			//see how much influence we want to give the rotation of the actor on the direction the camera is looking. This depends on the angle between the look direction and CP.
			//if player is looking almost all the way up or almost all the way down, it has full influence
			float camLerp = FMath::Cos(CP.RadiansToVector(camForward - DesiredUp * camForward.DistanceInDirection(tmpDesiredUp)));
			camLerp = CamSneakInfluence;
			//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Yellow, "raidans to vector: " + FString::SanitizeFloat(FMath::Abs(camForward.RadiansToVector(GetActorUpVector()) - PI)));
			if (camForward.DistanceInDirection(GetActorForwardVector()) <= 0) {
				camLerp = 0;
				GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, "fuck");
			}
			MyCamera->SetWorldRotation(FQuat::Slerp(q, MyCamera->GetComponentQuat(), FMath::Abs(camLerp)));
		}
	}

	//Setting movement speed
	if (ShadowSneak) {
		MovementComp->MoveType = UCustomMovement::MovementType::Sneak;
	}
	else {
		if (bSprint) MovementComp->MoveType = UCustomMovement::MovementType::Sprint;
		else if (bCrouch) MovementComp->MoveType = UCustomMovement::MovementType::Crouch;
		else MovementComp->MoveType = UCustomMovement::MovementType::Walk;
	}
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::SanitizeFloat(MovementComp->MovementSpeed));
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, "height: " + FString::SanitizeFloat(Capsule->GetScaledCapsuleHalfHeight()));
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, "radius: " + FString::SanitizeFloat(Capsule->GetScaledCapsuleRadius()));
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(addHeight));

	//interpolate capsule dimensions;
	MovementComp->HeightAdjustVel = FVector::ZeroVector;
	if (Capsule != nullptr && currentHeight != endHeight) {
		GetAddHeight();
		MovementComp->HeightAdjustVel += (RootComponent->GetUpVector() * addHeight * (addHeight > 0 ? 1 : 1.1));
		currentHeight += addHeight * DeltaTime;
		if (currentHeight == endHeight || addHeight > 0 ? currentHeight + addHeight * DeltaTime > endHeight : currentHeight + addHeight * DeltaTime < endHeight) {
			//MovementComp->AddInputVector(RootComponent->GetUpVector() * (endHeight - currentHeight));
			SetActorLocation(GetActorLocation() + RootComponent->GetUpVector() * (endHeight - currentHeight) * DeltaTime);
			currentHeight = endHeight;
			addHeight = 0;
		}
		Capsule->SetCapsuleRadius(NormalRadius);
		if (Capsule->GetScaledCapsuleRadius() > currentHeight) {
			Capsule->SetCapsuleRadius(currentHeight);
		}
		Capsule->SetCapsuleHalfHeight(currentHeight);
		RootComponent = Capsule;
		MyCamera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight / 100 * currentHeight));
	}
	Under = GetActorLocation();
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerPawn::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawn::LookUpAtRate);

	PlayerInputComponent->BindAction("Sneaky", IE_Pressed, this, &APlayerPawn::StartEndSneak);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerPawn::Jump);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerPawn::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerPawn::StopSprinting);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerPawn::CrouchControl);
}
UPawnMovementComponent* APlayerPawn::GetMovementComponent() const
{
	return MovementComp;
}
void APlayerPawn::MoveForward(float val) {
	if (MovementComp && MovementComp->UpdatedComponent == RootComponent) {
		//MovementComp->LateralVel += (GetActorForwardVector() * val * MovementComp->MovementSpeed);
		MovementComp->AddInputVector(GetActorForwardVector() * val * MovementComp->MovementSpeed);
	}
}
void APlayerPawn::MoveRight(float val) {
	if (MovementComp && MovementComp->UpdatedComponent == RootComponent) {
		//MovementComp->LateralVel += (GetActorRightVector() * val * MovementComp->MovementSpeed);
		MovementComp->AddInputVector(GetActorRightVector() * val * MovementComp->MovementSpeed);
	}
}
void APlayerPawn::TurnAtRate(float rate) {
	FVector newRight = RootComponent->GetRightVector().RotateAngleAxis(5 * rate, RootComponent->GetUpVector());
	FVector newForward = RootComponent->GetForwardVector().RotateAngleAxis(5 * rate, RootComponent->GetUpVector());
	RootComponent->SetWorldTransform(FTransform(newForward, newRight, RootComponent->GetUpVector(), GetActorLocation()));
}
void APlayerPawn::LookUpAtRate(float rate) {
	//MyCamera->SetRelativeRotation(MyCamera->RelativeRotation + FRotator(-rate, 0, 0));
	float addrot = 5 * rate;
	FRotator rot = MyCamera->GetRelativeRotation() + FRotator(-addrot, 0, 0);
	rot = FRotator(FMath::Clamp(rot.Pitch, -90.0f, 90.0f), rot.Yaw, rot.Roll);
	MyCamera->SetRelativeRotation(rot.Quaternion());
	if (MyCamera->GetRightVector() != GetActorRightVector()) {
		FVector newForward = MyCamera->GetForwardVector();
		FVector newUp = FVector::CrossProduct(newForward, GetActorRightVector());
		newUp.Normalize();
		if (newUp.DistanceInDirection(GetActorUpVector()) < 0) newUp = -FMath::Sign(MyCamera->GetForwardVector().DistanceInDirection(GetActorUpVector())) * GetActorForwardVector();
		//FQuat q = FTransform(newForward, GetActorRightVector(), newUp, MyCamera->GetComponentLocation()).GetRotation();
		MyCamera->SetWorldTransform(FTransform(newForward, GetActorRightVector(), newUp, MyCamera->GetComponentLocation()));
	}
}

void APlayerPawn::StartEndSneak()
{
	ShadowSneak = !ShadowSneak;
	startHeight = currentHeight;
	if (ShadowSneak) {
		endHeight = SneakHeight;
	}
	else {
		if (bCrouch) endHeight = CrouchHeight;
		else endHeight = NormalHeight;
	}
}

void APlayerPawn::Jump()
{
	MovementComp->Jump();
	if (ShadowSneak) StartEndSneak();
}

void APlayerPawn::Sprint()
{
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	if (MovementComp->CheckGrounded()) {
		startHeight = currentHeight;
		endHeight = NormalHeight;

		GetWorld()->LineTraceSingleByChannel(outHit, GetActorLocation(),
			GetActorLocation() + RootComponent->GetUpVector() * (NormalHeight + (NormalHeight - currentHeight)), ECC_Visibility, params);
		if (!outHit.bBlockingHit) {
			bBufferSprint = false;
			if (ShadowSneak) {
				StartEndSneak();
			}
			if (bCrouch) {
				StopCrouching();
			}
			bSprint = true;
			GetAddHeight();
		}
		else {
			endHeight = currentHeight;
			bBufferSprint = true;
		}
	}
	else {
		bBufferSprint = true;
	}
}

void APlayerPawn::StopSprinting() {
	bSprint = false;
	bBufferSprint = false;
}
void APlayerPawn::CrouchControl() {
	if (bCrouch) {
		StopCrouching();
	}
	else {
		Crouch();
	}
}
void APlayerPawn::Crouch() {
	StopSprinting();
	bCrouch = true;
	startHeight = currentHeight;
	endHeight = CrouchHeight;
	ShadowSneak = false;
	GetAddHeight();
}
void APlayerPawn::StopCrouching() {
	if (ShadowSneak) {
		StartEndSneak();
	}
	startHeight = currentHeight;
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	bool CanUnCrouch = true;
	GetWorld()->LineTraceSingleByChannel(outHit, GetActorLocation(),
		GetActorLocation() + RootComponent->GetUpVector() * (NormalHeight + (NormalHeight - currentHeight)), ECC_Visibility, params);
	if (!outHit.bBlockingHit || ShadowSneak) {
		endHeight = ShadowSneak ? SneakHeight : NormalHeight;
		GetAddHeight();
		bCrouch = false;
	}
}

void APlayerPawn::BufferEndCrouch()
{
}

void APlayerPawn::RootCollision(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UCapsuleComponent* cap = Cast<UCapsuleComponent>(HitComp);
	if (cap) {
		if ((SweepResult.ImpactPoint - (GetActorLocation() - cap->GetUpVector() * cap->GetScaledCapsuleHalfHeight_WithoutHemisphere())).DistanceInDirection(-cap->GetUpVector())
			>= FMath::Sin(25 * PI / 180) * cap->GetScaledCapsuleRadius()) {
		}
	}
}

void APlayerPawn::RootCollisionExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void APlayerPawn::RootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& SweepResult)
{
	UCapsuleComponent* cap = Cast<UCapsuleComponent>(HitComponent);
	if (cap) {
		if ((SweepResult.ImpactPoint - GetActorLocation()).DistanceInDirection(RootComponent->GetUpVector()) > cap->GetScaledCapsuleHalfHeight_WithoutHemisphere() + 2) {
			if (!(MovementComp->StartJump || MovementComp->EndJump) && addHeight > 0) {
				if (currentHeight > CrouchHeight) {
					Crouch();
				}
				else if (!ShadowSneak) {
					StartEndSneak();
				}
			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 0.5, FColor::Green, "true");
				MovementComp->StartJump = false;
				MovementComp->JumpVel = FVector(0, 0, 0);
				MovementComp->EndJump = true;
			}
		}
		if (HittingBottom(SweepResult.ImpactPoint, (ShadowSneak ? MovementComp->SneakMaxAngle : MovementComp->MaxAngle) + 1)) {
			/*FVector ThisNorm = SweepResult.ImpactNormal;
			ThisNorm.Normalize();
			FVector thisUnder = SweepResult.ImpactPoint;
			float angle = ThisNorm.RadiansToVector(GetActorUpVector());
			Grounded++;
			MovementComp->GroundNum = Grounded;
			//ThisNorm.RadiansToVector(GetActorUpVector()) <= MovementComp->MaxAngle * PI / 180 && 
			if (angle <= FloorAngle && angle <= (ShadowSneak ? MovementComp->SneakMaxAngle + 1 : MovementComp->MaxAngle + 1) * PI / 180) {
				//DrawDebugLine(GetWorld(), Under, Under + 100 * FloorNormal, FColor::Green, false, 1, 0, 1);
				
				FloorAngle = angle;//ThisNorm.RadiansToVector(GetActorUpVector());
				FloorNormal = ThisNorm;
				if (SweepResult.GetActor() != NULL) {
					if (SweepResult.GetActor()->IsA(AStairs::StaticClass())) {
						DesiredUp = SweepResult.GetActor()->GetActorUpVector();
						MovementComp->DownVel = -FloorNormal * 0.5;
					}
					else {
						if (SweepResult.GetActor()->FindComponentByClass<USneakIgnore>() == NULL) {
							DesiredUp = FloorNormal;
							MovementComp->DownVel = -FloorNormal * 0.5;
						}
						else {
							MovementComp->DownVel = FVector::ZeroVector;
						}
					}
				}
				else {
					DesiredUp = FloorNormal;
					MovementComp->DownVel = -FloorNormal * 0.5;
				}
				
			}*/
			FVector ThisNorm = SweepResult.ImpactNormal;
			ThisNorm.Normalize();
			Under = SweepResult.ImpactPoint;
			float angle = ThisNorm.RadiansToVector(GetActorUpVector());
			Grounded++;
			MovementComp->GroundNum = Grounded;
			//ThisNorm.RadiansToVector(GetActorUpVector()) <= MovementComp->MaxAngle * PI / 180 && 
			
			if (angle < FloorAngle && angle <= (MovementComp->MaxAngle) * PI / 180) {
				//DrawDebugLine(GetWorld(), Under, Under + 100 * FloorNormal, FColor::Green, false, 1, 0, 1);
				FloorAngle = angle;//ThisNorm.RadiansToVector(GetActorUpVector());
				if (!FVector::Coincident(FloorNormal, ThisNorm)) {
					OldNormal = FloorNormal;
				}
				FloorNormal = ThisNorm;
				FloorNormal.Normalize();
				MovementComp->DownVel = -FloorNormal * 30;
				if (SweepResult.GetActor() != NULL ? SweepResult.GetActor()->FindComponentByClass<USneakIgnore>() == NULL : true) {
					DesiredUp = FloorNormal;
					
				}
			}
		}
	}
}

bool APlayerPawn::HittingBottom(FVector hitPos, float maxDeg) {
	FVector center = GetActorLocation() - Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	return (center - hitPos).RadiansToVector(Capsule->GetUpVector()) <= maxDeg * PI / 180;
}

bool APlayerPawn::IsStepUp(FVector hitPos) {
	FVector center = GetActorLocation() - Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	return (hitPos - center).DistanceInDirection(-GetActorUpVector()) <= Capsule->GetScaledCapsuleRadius() - MovementComp->StepHeight;
}

bool APlayerPawn::CheckGrounded() {
	return ShadowSneak ? MovementComp->CheckGrounded() : MovementComp->GroundNum >= 1;
}

void APlayerPawn::GetAddHeight() {
	addHeight = (endHeight - startHeight) / HeightInterpTime / (FMath::Abs(startHeight - endHeight) / (NormalHeight - CrouchHeight));
	//UCapsuleComponent* A = Cast<UCapsuleComponent>(RootComponent);
}

APlayerPawn::Visibility APlayerPawn::PStealth(FVector location, float Attenuation, float candelas) {
	float mult = 0;
	Visibility ReturnVis;
	FHitResult outHit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	FVector Start;
	FVector End;
	//UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule != nullptr && (GetActorLocation() - location).Size() <= Attenuation) {
		float capsuleHeight = Capsule->GetScaledCapsuleHalfHeight();
		float capsuleRadius = Capsule->GetScaledCapsuleRadius();
		for (float f = -capsuleHeight; f <= capsuleHeight; f += capsuleHeight) {
			Start = GetActorLocation() + RootComponent->GetUpVector() * f;
			End = location;
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
			bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, End, Start, ECC_Visibility, CollisionParams);
			if (!outHit.bBlockingHit) {
				mult += 0.2;
				if (f == -capsuleHeight) {
					ReturnVis.GroundVis = candelas * 10000 / (FMath::Pow((Start - End).Size(), 2));
				}
			}
		}
		for (float f = -capsuleRadius; f <= capsuleRadius; f += 2 * capsuleRadius) {
			Start = RootComponent->GetRightVector() * f + GetActorLocation();
			End = location;
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
			bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, CollisionParams);
			if (!outHit.bBlockingHit) {
				mult += 0.2;
			}
		}
		//for the top bottom, left, right, and center of the player, do a line check. If there's something in the way subtract 0.2 from the multiplier cuz 5 points
		//if there's nothing in the don't do anything.
	}
	else {
		mult = 0;
		ReturnVis.GroundVis = 0;
	}
	ReturnVis.Vis = (2 * PI * (1 - FMath::Cos(PI)) * candelas * 10000) / (4 * PI * FMath::Pow((Start - End).Size(), 2)) * mult;
	return ReturnVis;
}
APlayerPawn::Visibility APlayerPawn::SStealth(FVector Spotlight, float inner, float outer, float Attenuation, FVector SpotAngle, float candelas) {
	Visibility ReturnVis;
	float mult = 0;
	float angle;
	FVector End;
	FVector Start = Spotlight;
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	//UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule != nullptr) {
		float halfHeight = Capsule->GetScaledCapsuleHalfHeight();
		float radius = Capsule->GetScaledCapsuleRadius();
		for (float f = -radius; f < radius; f += radius) {
			End = GetActorLocation() + GetActorRightVector() * f;
			angle = FMath::Acos(FVector::DotProduct(SpotAngle, (End - Spotlight)) / (SpotAngle.Size() * (End - Spotlight).Size())) * 180 / PI;
			GetWorld()->LineTraceSingleByChannel(outHit, Spotlight, End, ECC_Visibility, params);
			if (!outHit.bBlockingHit && angle <= outer && (End - Spotlight).Size() < Attenuation) {
				if (angle <= inner) {
					mult += 0.2;
				}
				else {
					mult += (outer - angle) / (outer - inner) * 0.2;
				}
			}
		}
		for (float f = -halfHeight; f < radius; f += 2 * halfHeight) {
			End = GetActorLocation() + GetActorUpVector() * f;
			angle = FMath::Acos(FVector::DotProduct(SpotAngle, (End - Spotlight)) / (SpotAngle.Size() * (End - Spotlight).Size())) * 180 / PI;
			GetWorld()->LineTraceSingleByChannel(outHit, Spotlight, End, ECC_Visibility, params);
			if (!outHit.bBlockingHit && angle <= outer && (End - Spotlight).Size() < Attenuation) {
				if (angle <= inner) {
					mult += 0.2;
				}
				else {
					mult += (outer - angle) / (outer - inner) * 0.2;
				}
				if (f == -halfHeight) {
					ReturnVis.GroundVis = (angle <= inner ? 1 : (outer - angle) / (outer - inner)) *
						(2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000) / FMath::Pow((Start - End).Size(), 2);
				}
			}
		}
		ReturnVis.Vis = mult * (2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000) / FMath::Pow((Start - End).Size(), 2);
	}
	else {
		ReturnVis.Vis = 0;
		ReturnVis.GroundVis = 0;
	}
	return ReturnVis;
}
APlayerPawn::Visibility APlayerPawn::DStealth(FVector Direction, float intensity, float length) {
	Visibility ReturnVis;
	float mult = 0;
	//UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	FVector End;
	FVector Start;
	FHitResult OutHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	if (Capsule != nullptr) {
		float Radius = Capsule->GetScaledCapsuleRadius();
		float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		for (float i = -Radius; i <= Radius; i += Radius) {
			Start = GetActorLocation() + RootComponent->GetRightVector() * i;
			End = Start - Direction * length;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
			if (!OutHit.bBlockingHit) {
				mult += 0.2;
			}
		}
		for (float j = -HalfHeight; j <= HalfHeight; j += 2 * HalfHeight) {
			Start = GetActorLocation() + RootComponent->GetUpVector() * j;
			End = Start - Direction * length;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
			if (!OutHit.bBlockingHit) {
				mult += 0.2;
				if (j == -HalfHeight) {
					ReturnVis.GroundVis = FMath::Square(intensity);
				}
			}
		}
	}
	ReturnVis.Vis = FMath::Square(mult * intensity);
	return ReturnVis;
}
void APlayerPawn::AddVis(Visibility vis) {
	MyVis.Vis += vis.Vis;
	MyVis.GroundVis += vis.GroundVis;
}
void APlayerPawn::SubVis(Visibility vis) {
	MyVis.Vis -= vis.Vis;
	MyVis.GroundVis -= vis.GroundVis;
}
float APlayerPawn::GetCapsuleVisibleArea() {
	//UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule) {
		return 2 * Capsule->GetScaledCapsuleRadius() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() + PI * FMath::Pow(Capsule->GetScaledCapsuleRadius(), 2);
	}
	return 0;
}