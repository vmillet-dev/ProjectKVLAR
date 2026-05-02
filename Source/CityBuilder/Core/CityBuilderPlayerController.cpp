// Fill out your copyright notice in the Description page of Project Settings.


#include "CityBuilderPlayerController.h"
#include "CityBuilderCameraPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

void ACityBuilderPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	bShowMouseCursor     = true;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void ACityBuilderPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);

	if (!EIC) return;

	EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ACityBuilderPlayerController::HandleMove);

	EIC->BindAction(IA_OrbitToggle, ETriggerEvent::Started,   this, &ACityBuilderPlayerController::HandleOrbitStarted);
	EIC->BindAction(IA_OrbitToggle, ETriggerEvent::Completed, this, &ACityBuilderPlayerController::HandleOrbitCompleted);

	EIC->BindAction(IA_OrbitLook, ETriggerEvent::Triggered, this, &ACityBuilderPlayerController::HandleOrbitLook);

	EIC->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &ACityBuilderPlayerController::HandleZoom);
	
	EIC->BindAction(IA_Select, ETriggerEvent::Started, this, &ThisClass::HandleSelect);
	EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &ThisClass::HandleTogglePause);
	EIC->BindAction(IA_Menu, ETriggerEvent::Started, this, &ThisClass::HandleOpenMenu);
}

// ─── Handlers ─────────────────────────────────────────────────────────────────

void ACityBuilderPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (ACityBuilderCameraPawn* CBPawn = GetCityBuilderPawn())
	{
		CBPawn->Move(Value);
	}
}

void ACityBuilderPlayerController::HandleOrbitStarted(const FInputActionValue& Value)
{
	GetMousePosition(SavedMouseX, SavedMouseY);
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (ACityBuilderCameraPawn* CBPawn = GetCityBuilderPawn())
	{
		CBPawn->StartOrbit();
	}
}

void ACityBuilderPlayerController::HandleOrbitCompleted(const FInputActionValue& Value)
{
	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	SetMouseLocation(FMath::RoundToInt(SavedMouseX), FMath::RoundToInt(SavedMouseY));

	if (ACityBuilderCameraPawn* CBPawn = GetCityBuilderPawn())
	{
		CBPawn->StopOrbit();
	}
}

void ACityBuilderPlayerController::HandleOrbitLook(const FInputActionValue& Value)
{
	if (ACityBuilderCameraPawn* CBPawn = GetCityBuilderPawn())
	{
		CBPawn->Orbit(Value);
	}
}

void ACityBuilderPlayerController::HandleZoom(const FInputActionValue& Value)
{
	if (ACityBuilderCameraPawn* CBPawn = GetCityBuilderPawn())
	{
		CBPawn->Zoom(Value);
	}
}

void ACityBuilderPlayerController::HandleSelect()
{
	UE_LOG(LogTemp, Warning, TEXT("Select clicked"));
	FHitResult Hit;

	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected: %s"), *Hit.GetActor()->GetName());
	}
}

void ACityBuilderPlayerController::HandleTogglePause()
{
	bool bPaused = UGameplayStatics::IsGamePaused(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("Pause toggled"));
	UGameplayStatics::SetGamePaused(GetWorld(), !bPaused);
}

void ACityBuilderPlayerController::HandleOpenMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Menu opened"));
}

// ─── Helper ───────────────────────────────────────────────────────────────────

ACityBuilderCameraPawn* ACityBuilderPlayerController::GetCityBuilderPawn() const
{
	return Cast<ACityBuilderCameraPawn>(GetPawn());
}
