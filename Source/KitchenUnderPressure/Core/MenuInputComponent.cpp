// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuInputComponent.h"
#include "MenuInputSubsystem.h"
#include "Settings/MenuInputSettings.h"
#include "UI/MenuNavWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"

void UMenuInputComponent::Setup(APlayerController* InPC)
{
	PC = InPC;
	if (!PC || bBound)
	{
		return;
	}

	const UMenuInputSettings* Settings = GetDefault<UMenuInputSettings>();
	if (!Settings)
	{
		return;
	}

	MenuContext = Settings->MenuContext.LoadSynchronous();
	UInputAction* NavigateAction = Settings->NavigateAction.LoadSynchronous();
	UInputAction* AdjustAction = Settings->AdjustAction.LoadSynchronous();
	UInputAction* AcceptAction = Settings->AcceptAction.LoadSynchronous();
	UInputAction* BackAction = Settings->BackAction.LoadSynchronous();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EIC)
	{
		return;
	}

	if (NavigateAction)
	{
		EIC->BindAction(NavigateAction, ETriggerEvent::Started, this, &UMenuInputComponent::OnNavigate);
	}
	if (AdjustAction)
	{
		EIC->BindAction(AdjustAction, ETriggerEvent::Started, this, &UMenuInputComponent::OnAdjust);
	}
	if (AcceptAction)
	{
		EIC->BindAction(AcceptAction, ETriggerEvent::Started, this, &UMenuInputComponent::OnAccept);
	}
	if (BackAction)
	{
		EIC->BindAction(BackAction, ETriggerEvent::Started, this, &UMenuInputComponent::OnBack);
	}

	if (const UGameInstance* GI = PC->GetGameInstance())
	{
		if (UMenuInputSubsystem* Sub = GI->GetSubsystem<UMenuInputSubsystem>())
		{
			Sub->OnDeviceChanged.AddUObject(this, &UMenuInputComponent::HandleDeviceChanged);
		}
	}

	bBound = true;
}

void UMenuInputComponent::ActivateMenu(UMenuNavWidget* Widget, int32 Priority)
{
	ActiveWidget = Widget;

	if (UEnhancedInputLocalPlayerSubsystem* EISub = GetEnhancedSubsystem())
	{
		if (MenuContext && !EISub->HasMappingContext(MenuContext))
		{
			EISub->AddMappingContext(MenuContext, Priority);
		}
	}

	// Sync the cursor and highlight to whatever device is currently active.
	if (PC)
	{
		if (const UGameInstance* GI = PC->GetGameInstance())
		{
			if (const UMenuInputSubsystem* Sub = GI->GetSubsystem<UMenuInputSubsystem>())
			{
				ApplyCursorVisibility(Sub->IsCursorVisible());
			}
		}
	}

	if (ActiveWidget)
	{
		ActiveWidget->RefreshFromDevice();
	}
}

void UMenuInputComponent::DeactivateMenu()
{
	if (UEnhancedInputLocalPlayerSubsystem* EISub = GetEnhancedSubsystem())
	{
		if (MenuContext)
		{
			EISub->RemoveMappingContext(MenuContext);
		}
	}

	ActiveWidget = nullptr;
}

void UMenuInputComponent::OnNavigate(const FInputActionValue& Value)
{
	if (ActiveWidget)
	{
		ActiveWidget->Navigate(Value.Get<float>());
	}
}

void UMenuInputComponent::OnAdjust(const FInputActionValue& Value)
{
	if (ActiveWidget)
	{
		ActiveWidget->Adjust(Value.Get<float>());
	}
}

void UMenuInputComponent::OnAccept()
{
	if (ActiveWidget)
	{
		ActiveWidget->Accept();
	}
}

void UMenuInputComponent::OnBack()
{
	if (ActiveWidget)
	{
		ActiveWidget->Back();
	}
}

void UMenuInputComponent::HandleDeviceChanged(EMenuInputDevice Device)
{
	// Only drive the cursor while a menu is actually active, so gameplay input
	// modes are left untouched.
	if (PC && ActiveWidget)
	{
		ApplyCursorVisibility(Device != EMenuInputDevice::Gamepad);
	}
}

void UMenuInputComponent::ApplyCursorVisibility(bool bVisible)
{
	if (!PC)
	{
		return;
	}

	PC->bShowMouseCursor = bVisible;

	// bShowMouseCursor is only read by the viewport on its next cursor query, which lags
	// a frame and can be overridden by a widget sitting under the cursor. Push the change
	// straight to the platform cursor so it hides/shows on the spot. SetType (not the
	// Windows ShowCursor counter) is used, so repeated calls are safe.
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetPlatformCursorVisibility(bVisible);
	}
}

UEnhancedInputLocalPlayerSubsystem* UMenuInputComponent::GetEnhancedSubsystem() const
{
	if (PC)
	{
		return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	}
	return nullptr;
}
