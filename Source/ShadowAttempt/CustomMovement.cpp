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
	StartJump = false;
	//i've written this out this way to make it more readable. multiplying both jump height and gravity by 100 because m to cm, 4 because of the way quadratic functions are
	JumpSpeed = FMath::Sqrt(100 * JumpHeight * 4 * 100 * Gravity);
}

void UCustomMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction); 
	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !Capsule || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	SetSpeed();
	if (GroundNum == 0) {
		if (StartJump) {
			StartJump = false;
			EndJump = true;
		}
	}
	else if(EndJump){
		StartJump = EndJump = false;
		JumpVel = FVector::ZeroVector;
	}
	//this is for the player pawn to reference. I can't guarantee what happens first, so CheckGrounded may not be what it's supposed to be when called from the pawn.
	bGroundedCache = CheckGrounded();
	
	LateralVel = ConsumeInputVector();
	LateralVel = LateralVel.GetClampedToMaxSize(MovementSpeed);
	//cache LateralVel because we're going to need to change and verify.
	CurrentLatVel = LateralVel;
	//make lateral vel perpendicular to the surface player's standing on.
	if (FVector::DotProduct(LateralVel, Pawn->FloorNormal) != 0) {
		FVector idk = FVector::CrossProduct(LateralVel, Pawn->FloorNormal);
		LateralVel = FVector::CrossProduct(idk, Pawn->FloorNormal);
		//make sure player's moving in the direction they want to be moving.
		if (LateralVel.DistanceInDirection(CurrentLatVel) < 0) LateralVel *= -1;
	}
	//if they're stepping down and not stepping up (not necessarily mutually exclusive), double downward velocity.
	if (CheckStepDown(LateralVel * DeltaTime) && !CheckStepUp(LateralVel * DeltaTime) && !StartJump && !EndJump) DownVel *= 2;

	FVector DesiredMovementThisFrame = (LateralVel + JumpVel + DownVel + HeightAdjustVel) * DeltaTime;

	FHitResult outHit;
	
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);

		// If we bumped into something, try to slide along it
		if (outHit.IsValidBlockingHit())
		{
			SlideAlongSurface(DesiredMovementThisFrame, 1.f - outHit.Time, outHit.Normal, outHit);
		}
	}

	//comment this if statement out if it doesn't work. chances are it won't, and it's fine as is.
	/*if (MoveType == MovementType::Sneak) {
		
		FCollisionQueryParams params;
		params.AddIgnoredActor(UpdatedComponent->GetOwner());
		FVector Start = UpdatedComponent->GetComponentLocation() + LateralVel * (Capsule->GetScaledCapsuleRadius() / LateralVel.Size());
		FVector End = Start + LateralVel * DeltaTime;
		GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, params);
		if (outHit.bBlockingHit) {
			if (UpdatedComponent->GetUpVector().RadiansToVector(outHit.ImpactNormal) <= ShadowMaxAngle * PI / 180) {
				Pawn->FloorNormal = outHit.ImpactNormal;
				Pawn->bMovementOverrideFloorNormal = true;
			}
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Emerald, "i feel like i've come up against a wall and i can't see behind it.");
			DrawDebugLine(GetWorld(), Start, End, FColor::Purple);
		}
	}*/
	//accelerate if it's not actually touching the ground. linecasts don't count.
	if (GroundNum == 0) {
		DownVel += (MoveType == MovementType::Sneak ? Pawn->FloorNormal : FVector::UpVector) * -Gravity * DeltaTime * 200;
	}
}
void UCustomMovement::SetSpeed()
{
	switch (MoveType) {
	case MovementType::Walk:
		MovementSpeed = NormalSpeed;
		break;
	case MovementType::Crouch:
		MovementSpeed = CrouchSpeed;
		break;
	case MovementType::Sprint:
		MovementSpeed = SprintSpeed;
		break;
	case MovementType::Sneak:
		MovementSpeed = SneakSpeed;
		break;
	default:
		MovementSpeed = NormalSpeed;
		break;
	}
}
void UCustomMovement::Jump() {
	if (CanJump()) {
		DownVel = FVector::ZeroVector;
		JumpVel = UpdatedComponent->GetUpVector() * (Pawn->ShadowSneak ? 200 * FMath::Sqrt(Gravity * (JumpHeight + (Pawn->NormalHeight - Pawn->SneakHeight)/100)) : JumpSpeed);
		StartJump = true;
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
		return GroundNum > 0 || Stepping || Hit.bBlockingHit || CheckStepDown(LateralVel / 60) || CheckStepUp(LateralVel / 60);
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
		return Hit.bBlockingHit && !StartJump;
	}
	return false;
}
bool UCustomMovement::CanJump() {
	if (Capsule) {
		FVector Start = Capsule->GetComponentLocation();
		FVector End = Start - Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + StepHeight);
		FCollisionQueryParams Params;
		FHitResult OutHit;
		Params.AddIgnoredActor(Pawn);
		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
		return OutHit.bBlockingHit || GroundNum > 0;
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
		return GroundNum > 0 || Hit.bBlockingHit;
	}
	return false;
}
;