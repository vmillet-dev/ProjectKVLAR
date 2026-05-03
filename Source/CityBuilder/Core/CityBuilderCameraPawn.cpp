// Fill out your copyright notice in the Description page of Project Settings.


#include "CityBuilderCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

ACityBuilderCameraPawn::ACityBuilderCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
 
	// ── Pivot au sol ──────────────────────────────────────────────────────────
	PivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	SetRootComponent(PivotComponent);
 
	// ── Spring Arm ────────────────────────────────────────────────────────────
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(PivotComponent);
	SpringArmComponent->TargetArmLength         = TargetArmLength;
	SpringArmComponent->bDoCollisionTest        = false;   // pas de collision pour une caméra city builder
	SpringArmComponent->bInheritPitch           = false;
	SpringArmComponent->bInheritRoll            = false;
	SpringArmComponent->bInheritYaw             = false;
 
	// Angle initial : vue à 45° depuis le dessus
	SpringArmComponent->SetRelativeRotation(FRotator(CurrentPitch, 0.f, 0.f));
 
	// ── Caméra ────────────────────────────────────────────────────────────────
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
}
 
void ACityBuilderCameraPawn::BeginPlay()
{
	Super::BeginPlay();
 
	// Synchronise le Yaw initial avec la rotation actuelle de l'acteur
	CurrentYaw       = GetActorRotation().Yaw;
	TargetArmLength  = SpringArmComponent->TargetArmLength;
}
 
void ACityBuilderCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
	// ── Zoom (interpolation douce) ────────────────────────────────────────────
	SpringArmComponent->TargetArmLength = FMath::FInterpTo(
		SpringArmComponent->TargetArmLength,
		TargetArmLength,
		DeltaTime,
		ZoomInterpSpeed
	);
}

// ─── Déplacement ──────────────────────────────────────────────────────────────
 
void ACityBuilderCameraPawn::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	const FRotator Rotation = SpringArmComponent->GetRelativeRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector MoveDirection = (Forward * Input.X) + (Right * -Input.Y);

	float SpeedMultiplier = 1.f;
	if (bScaleMoveSpeedWithZoom)
	{
		const float CurrentArmLength = SpringArmComponent->TargetArmLength;
		const float T = FMath::GetMappedRangeValueClamped(
			FVector2D(MinArmLength, MaxArmLength),
			FVector2D(0.f, 1.f),
			CurrentArmLength
		);
		SpeedMultiplier = FMath::Lerp(1.5f, 8.f, T * T);
	}

	AddActorWorldOffset(
		MoveDirection.GetSafeNormal() * MoveSpeed * SpeedMultiplier * GetWorld()->GetDeltaSeconds(),
		true
	);
}

 
// ─── Orbite ───────────────────────────────────────────────────────────────────
 
void ACityBuilderCameraPawn::StartOrbit()
{
	bOrbitActive = true;
}
 
void ACityBuilderCameraPawn::StopOrbit()
{
	bOrbitActive = false;
}
 
void ACityBuilderCameraPawn::Orbit(const FInputActionValue& Value)
{
	if (!bOrbitActive)
		return;
 
	const FVector2D LookDelta = Value.Get<FVector2D>();
 
	// Yaw : rotation horizontale libre, sans limite (360°)
	CurrentYaw = FMath::Fmod(CurrentYaw + LookDelta.X * OrbitYawSensitivity, 360.f);
 
	// Pitch : clampé pour ne jamais basculer sous le sol ni passer au-dessus du zénith
	CurrentPitch = FMath::Clamp(
		CurrentPitch + LookDelta.Y * OrbitPitchSensitivity,
		MinPitch,
		MaxPitch
	);
 
	SpringArmComponent->SetRelativeRotation(FRotator(CurrentPitch, CurrentYaw, 0.f));
}
 
// ─── Zoom ─────────────────────────────────────────────────────────────────────
 
void ACityBuilderCameraPawn::Zoom(const FInputActionValue& Value)
{
	const float ScrollAxis = Value.Get<float>();
 
	TargetArmLength = FMath::Clamp(
		TargetArmLength - ScrollAxis * ZoomStep,
		MinArmLength,
		MaxArmLength
	);
}
