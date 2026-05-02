// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CityBuilderPlayerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;

UCLASS()
class CITYBUILDER_API ACityBuilderPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Input Mapping
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_OrbitToggle;
 
	/** IA_OrbitLook — Axis2D (delta souris) */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_OrbitLook;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Zoom;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Select;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Pause;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Menu;

private:
	// ─── Handlers ─────────────────────────────────────────────────────────────
 
	void HandleMove(const FInputActionValue& Value);
 
	void HandleOrbitStarted(const FInputActionValue& Value);
	void HandleOrbitCompleted(const FInputActionValue& Value);
	void HandleOrbitLook(const FInputActionValue& Value);
 
	void HandleZoom(const FInputActionValue& Value);
	
	void HandleSelect();
	void HandleTogglePause();
	void HandleOpenMenu();
 
	// ─── Helpers ──────────────────────────────────────────────────────────────
 
	/** Retourne le pawn casté en ACityBuilderPawn, ou nullptr */
	class ACityBuilderCameraPawn* GetCityBuilderPawn() const;
	
	float SavedMouseX = 0.f;
	float SavedMouseY = 0.f;

};
