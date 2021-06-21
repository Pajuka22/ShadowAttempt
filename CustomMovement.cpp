#include "CustomMovement.h"
#include "CoreMinimal.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "PlayerPawn.h"

void UCustomMovement::BeginPlay() {
	Super::BeginPlay();
	Pawn = Cast<APlayerPawn>(PawnOwner);
	Capsule = Cast<UCapsuleComponent>(UpdatedComponent);
	//i've written this out this way to make it more readable. multiplying both jump height and gravity by 100 because m to cm, 4 because of the way quadratic functions are
	jumpSpeed = FMath::Sqrt(100 * JumpHeight * 4 * 100 * Gravity);
	moveSpeed = NormalSpeed;
	moveType = MovementType::Walk;
}

void UCustomMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction); 
	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !Capsule || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	if (groundNum > 0 && !startJump) SetSpeed();
	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Purple, FString::SanitizeFloat(MovementSpeed));
	if (groundNum == 0) {
		if (startJump) {
			startJump = false;
			endJump = true;
		}
	}
	else if(endJump){
		startJump = endJump = Jumping = false;
		JumpVel = FVector::ZeroVector;
	}
	
	lateralVel = ConsumeInputVector();
	lateralVel = lateralVel.GetClampedToMaxSize(MovementSpeed);
	//cache lateralVel because we're going to need to change and verify.
	CurrentLatVel = lateralVel;
	//make lateral vel perpendicular to the surface player's standing on.
	if (FVector::DotProduct(lateralVel, Pawn->FloorNormal) != 0) {
		FVector idk = FVector::CrossProduct(lateralVel, Pawn->FloorNormal);
		lateralVel = FVector::CrossProduct(idk, Pawn->FloorNormal);
		//make sure player's moving in the direction they want to be moving.
		if (lateralVel.DistanceInDirection(CurrentLatVel) < 0) lateralVel *= -1;
	}
	//if they're stepping down and not stepping up (not necessarily mutually exclusive), double downward velocity.
	if (CheckStepDown(lateralVel * DeltaTime) && !CheckStepUp(lateralVel * DeltaTime) && !startJump && !endJump) DownVel *= 2;
	//if (CheckStepUp(lateralVel * DeltaTime) && groundNum >= 1) DownVel = Capsule->GetUpVector() * GetStepHeight(lateralVel * DeltaTime) * 10;
	if (groundNum == 0) {
		DownVel += (MoveType == MovementType::Sneak ? Pawn->FloorNormal : FVector::UpVector) * -Gravity * DeltaTime * 200;
	}

	FVector desiredMovementThisFrame = (lateralVel + JumpVel + DownVel + HeightAdjustVel) * DeltaTime;

	FHitResult outHit;
	
	if (!desiredMovementThisFrame.IsNearlyZero())
	{
		SafeMoveUpdatedComponent(desiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);

		// If we bumped into something, try to slide along it
		if (outHit.IsValidBlockingHit())
		{
			if((outHit.ImpactPoint - UpdatedComponent->GetComponentLocation()).DistanceInDirection(LateralVel) >= distInMoveDir && (shadowSneak ? (HittingBottom(outHit.ImpactPoint) || HittingSides(outHit.ImpactPoint)) : HittingBottom(outHit.ImpactPoint, maxAngle))){
				floorNormal = outHit.ImpactNormal;
				if(shadowSneak && !FVector::Coincident(desiredUp, floorNormal)) angleAtStartRotate = outHit.ImpactNormal.RadiansToVector(desiredUp);
				desiredUp = shadowSneak ? outHit.ImpactNormal : FVector::UpVector;
				distInMoveDir = (outHit.ImpactPoint - UpdatedComponent->GetComponentLocation()).DistanceInDirection(LateralVel);
			}
			if(HittingBottom(outHit.ImpactPoint, 45, true)){
				jumpVel = FVector::ZeroVector;
				downVel = FVector::ZeroVector;
			}
			SlideAlongSurface(desiredMovementThisFrame, 1.f - outHit.Time, outHit.Normal, outHit);
		}
	}
	//accelerate if it's not actually touching the ground. linecasts don't count.
	RotatePlayer(DeltaTime);
	
}

void RotatePlayer(float DeltaTime){

	if (!FVector::Coincident(desiredUp, GetActorUpVector())) {
		if(playerCam) FVector camForward = playerCam->GetForwardVector();
		FVector newUp = tmpDesiredUp;
		FVector newRight;
		FVector newForward;
		if ((newUp ^ camForward).DistanceInDirection(GetActorRightVector()) > 0){// && (newUp.DistanceInDirection(GetActorForwardVector()) > 0)){
			newRight = newUp ^ camForward;
			newForward = newUp ^ newRight;
		}
		else {
			if (!FVector::Coincident(GetActorForwardVector(), newUp)) {
				newRight = newUp ^ GetActorForwardVector();
				newForward = newUp ^ newRight;
			}
			else {
				newForward = newUp ^ GetActorRightVector();
				newRight = newUp ^ newForward;
			}
		}
		//if((newUp ^ camForward).DistanceInDirection(GetActorRightVector()) > 0)
		newUp.Normalize();
		newRight.Normalize();
		newForward.Normalize();
		FQuat q = FTransform(newForward, newRight, newUp, FVector::ZeroVector).GetRotation();
		float angDist = q.AngularDistance(GetActorQuat());
		if (angDist <= MaxRotateSpeed * PI / 180 * DeltaTime * 
		(rotateSpeedByActualDifference ? rotateSpeedByActualDifference->GetFloatValue(angDist * 2 / PI) : 1) *
		(rotateSpeedByInitialDifference ? rotateSpeedByInitialDifference->GetFloatValue(angleAtStartRotate))) {
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, "ima just rotate all the way.");
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::White, "not rotating all the way");
			q = FQuat::Slerp(GetActorQuat(), q, MaxRotateSpeed * PI / 180 * DeltaTime * 
			(rotateSpeedByActualDifference ? rotateSpeedByActualDifference->GetFloatValue(angDist * 2 / PI) : 1)/ angDist) *
			(rotateSpeedByInitialDifference ? rotateSpeedByInitialDifference->GetFloatValue(angleAtStartRotate));
		}
		SetActorRotation(q);
		if(playerCam){
			float camLerp = camSneakInfluence;
			if (camForward.DistanceInDirection(GetActorForwardVector()) < 0) {
				camForward = FMath::Sign(camForward.DistanceInDirection(GetActorUpVector())) * GetActorUpVector();
				camLerp = 1;
			}
			if (camLerp < 1) {
				newUp = camForward ^ GetActorRightVector();
				q = FTransform(camForward, GetActorRightVector(), newUp, FVector::ZeroVector).GetRotation();
				playerCam->SetWorldRotation(FQuat::Slerp(q, playerCam->GetComponentQuat(), camLerp));
			}
		}
	}
}

void UCustomMovement::SetMoveType(MoveTypes inType){
	moveType = inType;
}

void UCustomMovement::SetSpeed()
{
	switch (moveType) {
	case MovementType::Walk:
		MovementSpeed = normalSpeed;
		break;
	case MovementType::Crouch:
		MovementSpeed = crouchSpeed;
		break;
	case MovementType::Sprint:
		MovementSpeed = sprintSpeed;
		break;
	case MovementType::Sneak:
		MovementSpeed = sneakSpeed;
		break;
	default:
		MovementSpeed = normalSpeed;
		break;
	}
}
void UCustomMovement::Jump() {
	if (CanJump()) {
		downVel = FVector::ZeroVector;
		jumpVel = UpdatedComponent->GetUpVector() * (Pawn->ShadowSneak ? 200 * FMath::Sqrt(Gravity * (JumpHeight + (Pawn->NormalHeight - Pawn->SneakHeight)/100)) : jumpSpeed);
		startJump = true;
		EndSneak();
	}
}
bool UCustomMovement::CheckGrounded() {
	if (Pawn) {
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() - UpdatedComponent->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + 5);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1);
		FCollisionQueryParams Params;
		FHitResult Hit;
		Params.AddIgnoredActor(UpdatedComponent->GetOwner());
		bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		return groundNum > 0 || Stepping || Hit.bBlockingHit || CheckStepDown(lateralVel / 60) || CheckStepUp(lateralVel / 60);
	}
	return false;
}

bool UCustomMovement::CheckStepUp(FVector movement) {
	//collision prep for linecast
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UpdatedComponent->GetOwner());
	//done with collision stuff

	//we want to check the outside of the capsule, so we need to know how far from the vertical axis we need to linecast from.
	float radius = (FMath::Pow(Capsule->GetScaledCapsuleRadius(), 2) - StepHeight);
	if (radius >= 0) radius = FMath::Sqrt(radius);
	movement += movement / movement.Size() * radius;
	//set start and end of linecast
	FVector Start = GetActorLocation() - Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() - StepHeight);
	FVector End = Start + movement;
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	//if there's nothing taller than the step, cast downward to the ground level to see if we actually want to step up.
	if (!Hit.bBlockingHit) {
		Start = End;
		End = Start - Capsule->GetUpVector() * (StepHeight - 1);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		
		return Hit.bBlockingHit;
	}
	//if there was something in the way return false.
	return false;
}

bool UCustomMovement::CheckStepDown(FVector movement) {
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UpdatedComponent->GetOwner());
	float radius = (FMath::Pow(Capsule->GetScaledCapsuleRadius(), 2) - StepHeight);
	if (radius >= 0) radius = FMath::Sqrt(radius);
	movement += movement / movement.Size() * radius;
	FVector Start = GetActorLocation() - Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight();
	FVector End = Start + movement;
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
	bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (!Hit.bBlockingHit) {
		Start = End;
		End = Start - Capsule->GetUpVector() * StepHeight;
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
		IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		return Hit.bBlockingHit && !startJump;
	}
	return false;
}
float UCustomMovement::GetStepHeight(FVector movement) {
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetPawnOwner());
	FVector Start = GetActorLocation() - Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() - StepHeight) + movement;
	FVector End = Start - Capsule->GetUpVector() * StepHeight;
	GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_EngineTraceChannel2, params);
	return StepHeight - outHit.Distance;
}
bool UCustomMovement::CanJump() {
	if (Capsule) {
		FVector Start = Capsule->GetComponentLocation();
		FVector End = Start - Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + StepHeight);
		FCollisionQueryParams Params;
		FHitResult OutHit;
		Params.AddIgnoredActor(Pawn);
		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
		return OutHit.bBlockingHit || groundNum > 0;
	}
	return false;
}
bool UCustomMovement::CheckEndJump() {
	if (Pawn) {
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() - UpdatedComponent->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + 5);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1);
		FCollisionQueryParams Params;
		FHitResult Hit;
		Params.AddIgnoredActor(UpdatedComponent->GetOwner());
		bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		return groundNum > 0 || Hit.bBlockingHit;
	}
	return false;
}
;