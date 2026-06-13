// Copyright Epic Games, Inc. All Rights Reserved.

#include "KitchenUnderPressureCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KitchenUnderPressure.h"
#include "KitchenUnderPressurePlayerController.h"
#include "Input/InputConfig.h"
#include "Interaction/PickupComponent.h"
#include "AbilitySystemComponent.h"
#include "AlchemistPlayerState.h"
#include "KUPGameplayTags.h"
#include "Attributes/AlchemistAttributeSet.h"
#include "Spells/SpellCasterComponent.h"
#include "Spells/SpellTypes.h"
#include "Spells/SpellRegistrySubsystem.h"
#include "Spells/ElementDefinition.h"
#include "AlchemistGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AKitchenUnderPressureCharacter::AKitchenUnderPressureCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// Body faces where the player looks, so an object held in front of the body tracks horizontal
	// aim on every machine (the capsule transform is replicated). This already matches first person
	// movement (DoMove is view-relative); set here so the carry attach point doesn't rely on the Blueprint.
	bUseControllerRotationYaw = true;

	// Attach point for carried objects: in front of the character, around chest height. It is a child
	// of the capsule (replicated transform), unlike the first person camera which is only oriented to
	// the aim on the local player's machine. Move it in the Blueprint to tune the hold position.
	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPoint"));
	HoldPoint->SetupAttachment(GetCapsuleComponent());
	HoldPoint->SetRelativeLocation(FVector(60.f, 0.f, 50.f));

	// Pickup/carry/throw of physics objects lives in its own replicated component.
	PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("PickupComponent"));

	// Two-hand spell casting lives in its own replicated component (grants/activates GAS abilities).
	SpellCaster = CreateDefaultSubobject<USpellCasterComponent>(TEXT("SpellCaster"));

	// Rarity auras: small emissive spheres flanking the body, coloured by each hand's element and shown
	// only for rare+ spells so allies can read a loadout at a glance. Engine primitives = no art needed.
	LeftHandAura = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandAura"));
	LeftHandAura->SetupAttachment(GetCapsuleComponent());
	LeftHandAura->SetRelativeLocation(FVector(40.f, -25.f, 20.f));
	LeftHandAura->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHandAura->SetCastShadow(false);
	LeftHandAura->SetVisibility(false);

	RightHandAura = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandAura"));
	RightHandAura->SetupAttachment(GetCapsuleComponent());
	RightHandAura->SetRelativeLocation(FVector(40.f, 25.f, 20.f));
	RightHandAura->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandAura->SetCastShadow(false);
	RightHandAura->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> AuraMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (AuraMesh.Succeeded())
	{
		LeftHandAura->SetStaticMesh(AuraMesh.Object);
		RightHandAura->SetStaticMesh(AuraMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AuraMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (AuraMaterial.Succeeded())
	{
		LeftHandAura->SetMaterial(0, AuraMaterial.Object);
		RightHandAura->SetMaterial(0, AuraMaterial.Object);
	}
}

void AKitchenUnderPressureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogKitchenUnderPressure, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system."), *GetNameSafe(this));
		return;
	}

	// Actions live in the controller's UInputConfig, so the input wiring sits in one asset
	// instead of loose pointers on the pawn. The controller owns the mapping context lifecycle.
	const AKitchenUnderPressurePlayerController* KUPController = Cast<AKitchenUnderPressurePlayerController>(GetController());
	const UInputConfig* Config = KUPController ? KUPController->GetInputConfig() : nullptr;
	if (!Config)
	{
		UE_LOG(LogKitchenUnderPressure, Warning, TEXT("'%s' has no InputConfig (assign it on the player controller); movement input is disabled."), *GetNameSafe(this));
		return;
	}

	// Jumping
	if (Config->JumpAction)
	{
		EnhancedInputComponent->BindAction(Config->JumpAction, ETriggerEvent::Started, this, &AKitchenUnderPressureCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(Config->JumpAction, ETriggerEvent::Completed, this, &AKitchenUnderPressureCharacter::DoJumpEnd);
	}

	// Moving
	if (Config->MoveAction)
	{
		EnhancedInputComponent->BindAction(Config->MoveAction, ETriggerEvent::Triggered, this, &AKitchenUnderPressureCharacter::MoveInput);
	}

	// Looking/Aiming
	if (Config->LookAction)
	{
		EnhancedInputComponent->BindAction(Config->LookAction, ETriggerEvent::Triggered, this, &AKitchenUnderPressureCharacter::LookInput);
	}
	if (Config->MouseLookAction)
	{
		EnhancedInputComponent->BindAction(Config->MouseLookAction, ETriggerEvent::Triggered, this, &AKitchenUnderPressureCharacter::LookInput);
	}

	// Pause / in-game menu (logic lives on the player controller)
	if (Config->PauseAction)
	{
		EnhancedInputComponent->BindAction(Config->PauseAction, ETriggerEvent::Started, this, &AKitchenUnderPressureCharacter::PauseInput);
	}

	// Interact: pick up the aimed object, or drop the held one (same key toggles)
	if (Config->InteractAction)
	{
		EnhancedInputComponent->BindAction(Config->InteractAction, ETriggerEvent::Started, this, &AKitchenUnderPressureCharacter::DoInteract);
	}

	// Throw: Started begins the charge, Completed releases it (same press/release split as Jump)
	if (Config->ThrowAction)
	{
		EnhancedInputComponent->BindAction(Config->ThrowAction, ETriggerEvent::Started, this, &AKitchenUnderPressureCharacter::DoThrowStart);
		EnhancedInputComponent->BindAction(Config->ThrowAction, ETriggerEvent::Completed, this, &AKitchenUnderPressureCharacter::DoThrowRelease);
	}

	// Spells: each mouse button casts its matching hand (left click = left hand).
	if (Config->CastLeftAction)
	{
		EnhancedInputComponent->BindAction(Config->CastLeftAction, ETriggerEvent::Started, this, &AKitchenUnderPressureCharacter::DoCastLeft);
	}
	if (Config->CastRightAction)
	{
		EnhancedInputComponent->BindAction(Config->CastRightAction, ETriggerEvent::Started, this, &AKitchenUnderPressureCharacter::DoCastRight);
	}
}


void AKitchenUnderPressureCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AKitchenUnderPressureCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AKitchenUnderPressureCharacter::PauseInput()
{
	// Menu ownership (widget, pause, input mode) lives on the player controller; the
	// character only forwards the input, mirroring the DoJump/DoMove delegation pattern.
	if (AKitchenUnderPressurePlayerController* PC = Cast<AKitchenUnderPressurePlayerController>(GetController()))
	{
		PC->ToggleInGameMenu();
	}
}

void AKitchenUnderPressureCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AKitchenUnderPressureCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AKitchenUnderPressureCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AKitchenUnderPressureCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AKitchenUnderPressureCharacter::DoInteract()
{
	// Carry logic (trace, attach, replication) lives in the pickup component.
	if (PickupComponent)
	{
		PickupComponent->Interact();
	}
}

void AKitchenUnderPressureCharacter::DoThrowStart()
{
	if (PickupComponent)
	{
		PickupComponent->StartThrowCharge();
	}
}

void AKitchenUnderPressureCharacter::DoThrowRelease()
{
	if (PickupComponent)
	{
		PickupComponent->ReleaseThrow();
	}
}

void AKitchenUnderPressureCharacter::DoCastLeft()
{
	// Left mouse button casts the left hand (direct mapping; supersedes the GDD 5.1 cross-wiring).
	if (SpellCaster)
	{
		SpellCaster->TryCast(EHand::Left);
	}
}

void AKitchenUnderPressureCharacter::DoCastRight()
{
	// Right mouse button casts the right hand.
	if (SpellCaster)
	{
		SpellCaster->TryCast(EHand::Right);
	}
}

UAbilitySystemComponent* AKitchenUnderPressureCharacter::GetAbilitySystemComponent() const
{
	// The ability system lives on the PlayerState, not the pawn.
	if (const AAlchemistPlayerState* PS = GetPlayerState<AAlchemistPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AKitchenUnderPressureCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// Server path: the PlayerState exists by now, so bind the ASC and apply starting attributes.
	InitAbilitySystem();
}

void AKitchenUnderPressureCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// Client path: the PlayerState just replicated; point the ASC's actor info at this pawn.
	InitAbilitySystem();
}

void AKitchenUnderPressureCharacter::InitAbilitySystem()
{
	AAlchemistPlayerState* PS = GetPlayerState<AAlchemistPlayerState>();
	if (!PS)
	{
		// Possession and PlayerState replication arrive in an unspecified order; the other callsite retries.
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Owner = PlayerState (persists across respawns), Avatar = this pawn.
	ASC->InitAbilityActorInfo(PS, this);

	// Keep the rarity auras in sync with the PlayerState-stored loadout. Bound here (not BeginPlay)
	// because on clients the PlayerState only exists once it has replicated; runs on every machine.
	if (!PS->OnHandSpellChanged.IsAlreadyBound(this, &AKitchenUnderPressureCharacter::OnHandSpellChanged))
	{
		PS->OnHandSpellChanged.AddDynamic(this, &AKitchenUnderPressureCharacter::OnHandSpellChanged);
	}
	UpdateHandAura(EHand::Left);
	UpdateHandAura(EHand::Right);

	if (HasAuthority())
	{
		// Apply starting attribute values once, from the init effect assigned in the Blueprint. If none
		// is assigned the attribute set's constructor defaults are used, which is fine for testing.
		if (!bAttributesInitialised && DefaultAttributesEffect)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DefaultAttributesEffect, 1.f, Context);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				bAttributesInitialised = true;
			}
		}

		// Bind the death signal once (server-only).
		if (!bDeathBound)
		{
			if (UAlchemistAttributeSet* Attributes = PS->GetAlchemistAttributeSet())
			{
				Attributes->OnOutOfHealth.AddUObject(this, &AKitchenUnderPressureCharacter::HandleDeath);
				bDeathBound = true;
			}
		}

		// Grant the starting spell abilities now that the ability system is bound to this pawn.
		if (SpellCaster)
		{
			SpellCaster->ServerInitializeAbilities();
		}
	}
}

void AKitchenUnderPressureCharacter::HandleDeath(AActor* DeadAvatar)
{
	if (DeadAvatar != this)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(KUPTags::State_Dead))
	{
		return; // already dead; ignore repeated zero-health signals
	}

	UE_LOG(LogKitchenUnderPressure, Log, TEXT("Player pawn '%s' died."), *GetNameSafe(this));

	if (ASC)
	{
		// TagOnly replication so clients (HUD, ally visuals) can also read the death state.
		ASC->AddLooseGameplayTag(KUPTags::State_Dead, 1, EGameplayTagReplicationState::TagOnly);
		ASC->CancelAllAbilities();
	}

	// Minimal v1 response (no respawn yet); tell the game mode so it can detect a party wipe.
	GetCharacterMovement()->DisableMovement();

	if (UWorld* World = GetWorld())
	{
		if (AAlchemistGameMode* GameMode = World->GetAuthGameMode<AAlchemistGameMode>())
		{
			GameMode->NotifyPlayerDied(GetController());
		}
	}
}

void AKitchenUnderPressureCharacter::KUP_DamageSelf(float Amount)
{
	// Console exec runs on the owning client; route to the server so damage stays authoritative.
	ServerDebugDamageSelf(Amount);
}

void AKitchenUnderPressureCharacter::ServerDebugDamageSelf_Implementation(float Amount)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !DebugDamageEffect)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DebugDamageEffect, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(KUPTags::Data_Damage, Amount);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void AKitchenUnderPressureCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AKitchenUnderPressureCharacter::OnHandSpellChanged(EHand Hand)
{
	UpdateHandAura(Hand);
}

void AKitchenUnderPressureCharacter::UpdateHandAura(EHand Hand)
{
	UStaticMeshComponent* Aura = (Hand == EHand::Left) ? LeftHandAura : RightHandAura;
	if (!Aura || !SpellCaster)
	{
		return;
	}

	const FSpellDefinition& Spell = SpellCaster->GetSpell(Hand);

	// Common spells (and empty hands) show no aura; rarer spells glow brighter and bigger.
	if (!Spell.IsValid() || Spell.Rarity == ESpellRarity::Common)
	{
		Aura->SetVisibility(false);
		return;
	}

	FLinearColor Color = FLinearColor::White;
	if (USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this))
	{
		if (const UElementDefinition* Element = Registry->FindElement(Spell.Element))
		{
			Color = Element->Color;
		}
	}

	float Scale = 0.15f;
	switch (Spell.Rarity)
	{
	case ESpellRarity::Epic:      Scale = 0.22f; break;
	case ESpellRarity::Legendary: Scale = 0.30f; break;
	default:                      Scale = 0.15f; break; // Rare
	}

	Aura->SetVisibility(true);
	Aura->SetRelativeScale3D(FVector(Scale));

	TObjectPtr<UMaterialInstanceDynamic>& MID = (Hand == EHand::Left) ? LeftAuraMID : RightAuraMID;
	if (!MID)
	{
		if (UMaterialInterface* BaseMaterial = Aura->GetMaterial(0))
		{
			MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (MID)
			{
				Aura->SetMaterial(0, MID);
			}
		}
	}
	if (MID)
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	}
}
