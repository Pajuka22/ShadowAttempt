// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "Runtime/HeadMountedDisplay/Public/MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "CustomMovement.h"
#include "Runtime/Engine/Classes/GameFramework/FloatingPawnMovement.h"
#include "Runtime/Core/Public/Math/Vector.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Core/Public/Math/TransformNonVectorized.h"
#include "Runtime/Engine/Classes/Curves/CurveVector.h"
#include "Stairs.h"
#include "SneakOverride.h"
#include "SneakIgnore.h"
#include "SneakCamOverride.h"
#include "PlayerVisibility.h"
#include "Blueprint/UserWidget.h"
#include "MyGameInstance.h"

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
	Capsule->OnComponentHit.AddDynamic(this, &APlayerPawn::RootHit);
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

	startHeight = Capsule->GetScaledCapsuleHalfHeight();
	currentHeight = startHeight;
	endHeight = NormalHeight;
	DesiredUp = FVector::UpVector;
	gameInstance = Cast<UMyGameInstance>(GetGameInstance());
}

void APlayerPawn::EndPlay(EEndPlayReason::Type Reason)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "game ended");
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	PlayerInputComponent->BindAction("Reset", IE_Pressed, this, &APlayerPawn::ResetLevel);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerPawn::Pause).bExecuteWhenPaused = true;
}

void APlayerPawn::Pause() {

	if (wPause) // Check if the Asset is assigned in the blueprint.
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GWorld, 0);
		if (isPaused) {
			//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, "unpausing... theoretically");
			if(pauseMenu) pauseMenu->RemoveFromParent();
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, "Pausing");
			// Create the widget and store it.
			pauseMenu = CreateWidget<UUserWidget>(GetWorld(), wPause);

			// now you can use the widget directly since you have a referance for it.
			// Extra check to  make sure the pointer holds the widget.
			if (pauseMenu)
			{
				//let add it to the view port
				pauseMenu->AddToViewport();
			}

			
			//AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController);

			//Show the Cursor.
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeGameAndUI());
		}
	}
	isPaused = !isPaused;

	UGameplayStatics::SetGamePaused(this, isPaused);
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
	if (gameInstance != NULL) {
		rate *= gameInstance->LookScaleX;
		GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Blue, FString::SanitizeFloat(gameInstance->LookScaleX));
	}
	FVector newRight = RootComponent->GetRightVector().RotateAngleAxis(rate, RootComponent->GetUpVector());
	FVector newForward = RootComponent->GetForwardVector().RotateAngleAxis(rate, RootComponent->GetUpVector());
	RootComponent->SetWorldTransform(FTransform(newForward, newRight, RootComponent->GetUpVector(), GetActorLocation()));
}
void APlayerPawn::LookUpAtRate(float rate) {
	if (gameInstance != NULL) rate *= gameInstance->LookScaleY;
	//MyCamera->SetRelativeRotation(MyCamera->RelativeRotation + FRotator(-rate, 0, 0));
	float addrot = rate;
	FRotator rot = MyCamera->GetRelativeRotation() + FRotator(-addrot, 0, 0);
	rot = FRotator(FMath::Clamp(rot.Pitch, -90.0f, 90.0f), rot.Yaw, rot.Roll);
	MyCamera->SetRelativeRotation(rot.Quaternion());
	if (MyCamera->GetRightVector() != GetActorRightVector()) {
		FVector newForward = MyCamera->GetForwardVector();
		FVector newUp = newForward ^ GetActorRightVector();
		newUp.Normalize();
		if (newUp.DistanceInDirection(GetActorUpVector()) < 0) newUp = -FMath::Sign(MyCamera->GetForwardVector().DistanceInDirection(GetActorUpVector())) * GetActorForwardVector();
		//FQuat q = FTransform(newForward, GetActorRightVector(), newUp, MyCamera->GetComponentLocation()).GetRotation();
		MyCamera->SetWorldTransform(FTransform(newForward, GetActorRightVector(), newUp, MyCamera->GetComponentLocation()));
	}
}

void APlayerPawn::StartEndSneak()
{
	/*ShadowSneak = !ShadowSneak;
	startHeight = currentHeight;
	if (ShadowSneak) {
		endHeight = SneakHeight;
	}
	else {
		if (bCrouch) endHeight = CrouchHeight;
		else endHeight = NormalHeight;
	}*/
	if (!ShadowSneak) {
		StartSneak();
	}
	else {
		EndSneak();
	}
}

void APlayerPawn::StartSneak() {
	if (CheckGrounded()) {
		startHeight = currentHeight;
		endHeight = SneakHeight;
		//notGroundedTime = 0;
	}
	else if (SneakBuffer < 0) SneakBuffer = MaxSneakBuffer;
	bCrouch = false;
	ShadowSneak = true;
}
void APlayerPawn::EndSneak() {
	ShadowSneak = false;
	startHeight = currentHeight;
	endHeight = bCrouch ? CrouchHeight : NormalHeight;
	//if (!CheckGrounded() && MovementComp->JumpVel.IsNearlyZero()) {
		//MovementComp->DownVel = FVector::ZeroVector;
	//}
	FloorNormal = FVector::UpVector;
	DesiredUp = FVector::UpVector;
}

void APlayerPawn::Jump()
{
	MovementComp->Jump();
	if (ShadowSneak) EndSneak();
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
	if(HitComponent == Capsule)
}

bool APlayerPawn::HittingBottom(FVector hitPos, float maxDeg, bool top) {
	FVector center = GetActorLocation() + (top ? 1 : -1) * Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	if (top) {
		return (hitPos - center).RadiansToVector(GetActorUpVector()) <= maxDeg * PI / 180;
	}
	return (center - hitPos).RadiansToVector(Capsule->GetUpVector()) <= maxDeg * PI / 180;
}

bool APlayerPawn::HittingSides(FVector hitPos) {
	return abs((GetActorLocation() - hitPos).DistanceInDirection(GetActorUpVector())) <= Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
}

bool APlayerPawn::IsStepUp(FVector hitPos, FVector hitNormal) {
	FVector center = GetActorLocation() - Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	return (center - hitPos).RadiansToVector(hitNormal) > 5 * PI / 180;
}

bool APlayerPawn::CheckGrounded() {
	return ShadowSneak ? MovementComp->CheckGrounded() : MovementComp->GroundNum >= 1;
}

void APlayerPawn::GetAddHeight() {
	addHeight = (endHeight - startHeight) / HeightInterpTime / (FMath::Abs(startHeight - endHeight) / (NormalHeight - CrouchHeight));
	//UCapsuleComponent* A = Cast<UCapsuleComponent>(RootComponent);
}

//i'm only going to comment this one in depth because the rest do basically the same thing.
PlayerVisibility APlayerPawn::PStealth(FVector location, float Attenuation, float candelas) {
	//i'll do 6 points
	float mult, g = 0;
	FHitResult outHit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	FVector Start;
	if (Capsule != nullptr && (GetActorLocation() - location).Size() <= Attenuation) {
		float x, y = 0;
		//for the top bottom, left, right, and center of the player, do a line check. If there's something in the way subtract 0.2 from the multiplier cuz 5 points
		//if there's nothing in the don't do anything.
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				//dont' check corners or center
				if ((i == 0) == (j == 0)) continue;

				x = i * Capsule->GetScaledCapsuleRadius();
				y = j * Capsule->GetScaledCapsuleHalfHeight();
				Start = GetActorLocation() + GetActorUpVector() * y + GetActorRightVector() * x;
				GetWorld()->LineTraceSingleByChannel(outHit, location, Start, ECC_Visibility, CollisionParams);
				if (!outHit.bBlockingHit) {
					//DrawDebugLine(GetWorld(), Start, location, FColor::Green, false, 1/60, 0, 1);
					++mult;
					if (j == -1) {
						g = candelas * 10000 / (FMath::Pow((Start - location).Size(), 2));
					}
				}
				//else DrawDebugLine(GetWorld(), outHit.Location, location, FColor::Red, false, 1/60, 0, 1);
				
				if (j != 0 || i == 0) continue;
				
				Start = GetActorLocation() + GetActorUpVector() * y + GetActorForwardVector() * x;
				GetWorld()->LineTraceSingleByChannel(outHit, location, Start, ECC_Visibility, CollisionParams);
				if (!outHit.bBlockingHit) {
					//DrawDebugLine(GetWorld(), Start, location, FColor::Green, false, 1/60, 0, 1);
					++mult;
				}
				//else DrawDebugLine(GetWorld(), outHit.Location, location, FColor::Red, false, 1/60, 0, 1);
			}
		}

	}
	else return PlayerVisibility(0, 0);
	return PlayerVisibility((2 * PI * (1 - FMath::Cos(PI)) * candelas * 10000) / (4 * PI * FMath::Pow((Start - location).Size(), 2)) * mult/6, g);
}
PlayerVisibility APlayerPawn::SStealth(FVector location, float inner, float outer, float Attenuation, FVector SpotAngle, float candelas) {
	float g = 0;
	float mult = 0;
	//UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule != nullptr && (GetActorLocation() - location).Size() <= Attenuation) {
		float angle;
		FVector Start;
		FHitResult outHit;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		float x, y;
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				if ((i == 0) == (j == 0)) continue;
				x = i * Capsule->GetScaledCapsuleRadius();
				y = j * Capsule->GetScaledCapsuleHalfHeight();
				Start = GetActorLocation() + GetActorRightVector() * x + GetActorUpVector() * y;
				angle = SpotAngle.RadiansToVector(Start - location) * 180 / PI;
				if (angle <= outer) {
					GetWorld()->LineTraceSingleByChannel(outHit, Start, location, ECC_Visibility, params);
					//DrawDebugLine(GetWorld(), Start, location, FColor::Blue, false, 1 / 60, 0, 1);
					if (!outHit.bBlockingHit) {
						if (angle <= inner) {
							++mult;
						}
						else {
							mult += (outer - angle) / (outer - inner);
						}
						if (j == -1) {
							g = (angle <= inner ? 1 : (outer - angle) / (outer - inner)) *
								(2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000) / FMath::Pow((location - Start).Size(), 2);
						}
					}
				}
				if (j != 0) continue;

				Start = GetActorLocation() + GetActorForwardVector() * x;
				angle = SpotAngle.RadiansToVector(Start - location) * 180 / PI;
				if (angle <= outer) {
					//DrawDebugLine(GetWorld(), Start, location, FColor::Blue, false, 1 / 60, 0, 1);
					GetWorld()->LineTraceSingleByChannel(outHit, Start, location, ECC_Visibility, params);
					if (!outHit.bBlockingHit) {
						if (angle <= inner) {
							++mult;
						}
						else {
							mult += (outer - angle) / (outer - inner);
						}
					}
				}
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Blue, FString::SanitizeFloat(mult));
		return PlayerVisibility(mult / 6 * (2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000) / FMath::Pow((GetActorLocation() - location).Size(), 2), g);
	}
	return PlayerVisibility(0, 0);
	//return PlayerVisibility(mult/6 * (2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000) / FMath::Pow((GetActorLocation() - location).Size(), 2), g);
}
PlayerVisibility APlayerPawn::DStealth(FVector direction, float intensity, float length) {
	float g = 0;
	float mult = 0;
	if (Capsule != nullptr) {
		FVector End;
		FVector Start;
		FHitResult OutHit;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				if ((i == 0) == (j == 0)) continue;
				Start = GetActorLocation() + i * Capsule->GetScaledCapsuleRadius() * GetActorRightVector() + j * Capsule->GetScaledCapsuleHalfHeight() * GetActorUpVector();
				End = Start - direction * length;
				GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
				if (!OutHit.bBlockingHit) {
					++mult;
					if (j == -1) {
						g = FMath::Square(intensity);
					}
				}
				
				if (j != 0) continue;

				Start = GetActorLocation() + i * Capsule->GetScaledCapsuleRadius() * GetActorForwardVector() + j * Capsule->GetScaledCapsuleHalfHeight() * GetActorUpVector();
				End = Start - direction * length;
				GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
				if (!OutHit.bBlockingHit) {
					++mult;
					if (j == -1) {
						g = FMath::Square(intensity);
					}
				}
			}
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Red, FString::FromInt(mult));
	return PlayerVisibility(FMath::Square(mult/6 * intensity), g);
}
float APlayerPawn::GetCapsuleVisibleArea() {
	//UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule) {
		return 2 * Capsule->GetScaledCapsuleRadius() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() + PI * FMath::Pow(Capsule->GetScaledCapsuleRadius(), 2);
	}
	return 0;
}
void APlayerPawn::ResetLevel() {
	UGameplayStatics::OpenLevel(this,  FName(*GetWorld()->GetName()), false);
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, "Reset level");
}