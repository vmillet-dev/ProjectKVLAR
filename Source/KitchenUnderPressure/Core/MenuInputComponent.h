// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MenuInputComponent.generated.h"

class APlayerController;
class UMenuNavWidget;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
struct FInputActionValue;
enum class EMenuInputDevice : uint8;

/**
 * Lives on a player controller. Binds the menu navigation actions (from UMenuInputSettings)
 * once, forwards them to whichever UMenuNavWidget is active, adds/removes the menu mapping
 * context, and hides the mouse cursor while the player is on the gamepad.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UMenuInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Cache the controller, bind the three actions, and subscribe to device changes.
	void Setup(APlayerController* InPC);

	// Make Widget the navigation target and add the menu mapping context.
	void ActivateMenu(UMenuNavWidget* Widget, int32 Priority = 1);

	// Remove the menu mapping context and stop forwarding input.
	void DeactivateMenu();

private:
	void OnNavigate(const FInputActionValue& Value);
	void OnAdjust(const FInputActionValue& Value);
	void OnAccept();
	void OnBack();
	void HandleDeviceChanged(EMenuInputDevice Device);

	// Sets bShowMouseCursor and forces Slate to apply it immediately (no one-frame lag).
	void ApplyCursorVisibility(bool bVisible);

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedSubsystem() const;

	UPROPERTY()
	TObjectPtr<APlayerController> PC;

	UPROPERTY()
	TObjectPtr<UMenuNavWidget> ActiveWidget;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MenuContext;

	bool bBound = false;
};
