// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/PickupComponent.h"
#include "Interaction/PickupableActor.h"
#include "KitchenUnderPressureCharacter.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/ThrowChargeWidget.h"
#include "Net/UnrealNetwork.h"

UPickupComponent::UPickupComponent()
{
	// Tick is enabled only while charging a throw (see StartThrowCharge / ReleaseThrow).
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UPickupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPickupComponent, HeldActor);
}

void UPickupComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Don't leave a cube attached to a pawn that is being destroyed.
	if (HeldActor && GetOwner() && GetOwner()->HasAuthority())
	{
		Release(FVector::ZeroVector);
	}

	if (ChargeWidget)
	{
		ChargeWidget->RemoveFromParent();
		ChargeWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCharging && ChargeWidget)
	{
		ChargeWidget->SetChargePercent(GetChargeRatio());
	}
}

void UPickupComponent::Interact()
{
	// HeldActor is replicated to the owning client, so it can decide pickup vs. drop locally.
	if (HeldActor)
	{
		ServerDrop();
	}
	else
	{
		ServerTryPickup();
	}
}

void UPickupComponent::StartThrowCharge()
{
	if (!HeldActor)
	{
		return;
	}

	bCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	// Show and start feeding the HUD charge bar (local owning client only).
	EnsureChargeWidget();
	if (ChargeWidget)
	{
		ChargeWidget->SetChargePercent(0.f);
		ChargeWidget->Show();
	}
	SetComponentTickEnabled(true);
}

void UPickupComponent::ReleaseThrow()
{
	if (!bCharging)
	{
		return;
	}

	// Read the charge before clearing the flag, then stop feeding and hide the HUD bar.
	const float ChargeRatio = GetChargeRatio();
	bCharging = false;
	SetComponentTickEnabled(false);
	if (ChargeWidget)
	{
		ChargeWidget->Hide();
	}

	if (!HeldActor)
	{
		return;
	}

	FVector AimLocation, AimDirection;
	GetAimPoint(AimLocation, AimDirection);

	ServerThrow(FVector_NetQuantizeNormal(AimDirection), ChargeRatio);
}

void UPickupComponent::ServerTryPickup_Implementation()
{
	if (HeldActor)
	{
		return;
	}

	if (APickupableActor* Target = TraceForPickup())
	{
		Pickup(Target);
	}
}

void UPickupComponent::ServerDrop_Implementation()
{
	if (HeldActor)
	{
		Release(FVector::ZeroVector);
	}
}

void UPickupComponent::ServerThrow_Implementation(FVector_NetQuantizeNormal Direction, float ChargeRatio)
{
	if (!HeldActor)
	{
		return;
	}

	// Clamp the client-supplied charge on the server before turning it into a speed.
	const float Clamped = FMath::Clamp(ChargeRatio, 0.f, 1.f);
	const float Speed = FMath::Lerp(MinThrowSpeed, MaxThrowSpeed, Clamped);

	Release(Direction.GetSafeNormal() * Speed);
}

APickupableActor* UPickupComponent::TraceForPickup() const
{
	FVector Start, Direction;
	GetAimPoint(Start, Direction);
	const FVector End = Start + Direction * ReachDistance;

	FCollisionQueryParams Params(FName(TEXT("PickupTrace")), /*bTraceComplex=*/false, GetOwner());

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, Start, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius), Params);

	if (!bHit)
	{
		return nullptr;
	}

	APickupableActor* Pickupable = Cast<APickupableActor>(Hit.GetActor());
	return (Pickupable && !Pickupable->IsHeld()) ? Pickupable : nullptr;
}

void UPickupComponent::Pickup(APickupableActor* Target)
{
	if (!Target)
	{
		return;
	}

	// Attach to the character's hold point (in front of the body) so every client sees the carried
	// object in the same place. The first person camera is unreliable here: it is only rotated to the
	// aim on the local player's machine, so attaching to it puts the object above the head elsewhere.
	USceneComponent* AttachPoint = nullptr;
	if (const AKitchenUnderPressureCharacter* Character = Cast<AKitchenUnderPressureCharacter>(GetOwner()))
	{
		AttachPoint = Character->GetHoldPoint();
	}
	if (!AttachPoint && GetOwner())
	{
		AttachPoint = GetOwner()->GetRootComponent();
	}

	HeldActor = Target;
	Target->OnPickedUp(AttachPoint);
}

void UPickupComponent::Release(const FVector& LaunchVelocity)
{
	if (!HeldActor)
	{
		return;
	}

	HeldActor->OnReleased(LaunchVelocity);
	HeldActor = nullptr;
}

void UPickupComponent::GetAimPoint(FVector& OutLocation, FVector& OutDirection) const
{
	OutLocation = FVector::ZeroVector;
	OutDirection = FVector::ForwardVector;

	// GetActorEyesViewPoint uses the controller's view (control rotation), which is replicated to
	// the server for the owning pawn, so the aim is accurate on both client and server.
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
		OutLocation = ViewLocation;
		OutDirection = ViewRotation.Vector();
	}
}

float UPickupComponent::GetChargeRatio() const
{
	if (!bCharging)
	{
		return 0.f;
	}

	const float HeldTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	return FMath::Clamp(HeldTime / FMath::Max(MaxChargeTime, 0.01f), 0.f, 1.f);
}

void UPickupComponent::EnsureChargeWidget()
{
	if (ChargeWidget || !ChargeWidgetClass)
	{
		return;
	}

	// HUD only exists for the local player driving this pawn.
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
	{
		ChargeWidget = CreateWidget<UThrowChargeWidget>(PC, ChargeWidgetClass);
		if (ChargeWidget)
		{
			ChargeWidget->AddToViewport();
		}
	}
}
