// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "System/KUPCombatStatics.h"
#include "Enemies/EnemyCharacter.h"
#include "KUPGameplayTags.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

ASpellProjectile::ASpellProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(14.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	SetRootComponent(CollisionSphere);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CollisionSphere);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(0.3f));

	// Default to engine primitives so a spell projectile needs no authored asset. BasicShapeMaterial
	// exposes a "Color" parameter we tint per element.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SphereMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (SphereMaterial.Succeeded())
	{
		MeshComp->SetMaterial(0, SphereMaterial.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f; // straight line for the first slice
	ProjectileMovement->InitialSpeed = 0.f;            // velocity is set in InitProjectile
	ProjectileMovement->MaxSpeed = 0.f;                // 0 = uncapped

	InitialLifeSpan = 5.f; // self-destruct if it never hits anything
}

void ASpellProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpellProjectile, Color);
	DOREPLIFETIME(ASpellProjectile, VisualScale);
}

void ASpellProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisual();

	if (HasAuthority())
	{
		CollisionSphere->OnComponentHit.AddDynamic(this, &ASpellProjectile::OnSphereHit);
		if (AActor* ProjInstigator = GetInstigator())
		{
			CollisionSphere->IgnoreActorWhenMoving(ProjInstigator, true);
		}
	}
}

void ASpellProjectile::InitProjectile(const FVector& Velocity, float InDamage, TSubclassOf<UGameplayEffect> InDamageEffect, TSubclassOf<UGameplayEffect> InStatusEffect, UAbilitySystemComponent* InInstigatorASC, const FLinearColor& InColor, float InVisualScale, const FGameplayTag& InElement)
{
	Damage = InDamage;
	DamageEffect = InDamageEffect;
	StatusEffect = InStatusEffect;
	InstigatorASC = InInstigatorASC;
	Element = InElement;

	Color = InColor;             // replicated; OnRep tints/scales the projectile on clients
	VisualScale = InVisualScale;
	ApplyVisual();               // apply immediately on the server

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Velocity;
	}
}

void ASpellProjectile::OnSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!OtherActor || OtherActor == GetInstigator() || OtherActor == GetOwner())
	{
		return;
	}

	// Apply damage + status to anything with an ability system; world geometry just stops us. The
	// friendly-fire gate skips the damage on allies (the projectile still bursts on them).
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		if (InstigatorASC.IsValid() && UKUPCombatStatics::CanDamage(InstigatorASC.Get(), TargetASC))
		{
			if (DamageEffect)
			{
				FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
				Context.AddInstigator(GetInstigator(), this);
				const FGameplayEffectSpecHandle Spec = InstigatorASC->MakeOutgoingSpec(DamageEffect, 1.f, Context);
				if (Spec.IsValid())
				{
					Spec.Data->SetSetByCallerMagnitude(KUPTags::Data_Damage, Damage);
					InstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
				}
			}

			if (StatusEffect)
			{
				FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
				Context.AddInstigator(GetInstigator(), this);
				const FGameplayEffectSpecHandle Spec = InstigatorASC->MakeOutgoingSpec(StatusEffect, 1.f, Context);
				if (Spec.IsValid())
				{
					InstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
				}
			}

			// Report the elemental hit so a second player's opposed element can react (GDD 8.1).
			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor))
			{
				Enemy->NotifyElementalHit(InstigatorASC.Get(), Element);
			}
		}
	}

	Destroy();
}

void ASpellProjectile::OnRep_Visual()
{
	ApplyVisual();
}

void ASpellProjectile::ApplyVisual()
{
	if (!MeshComp)
	{
		return;
	}

	// Power-driven size: bigger, brighter projectiles read as stronger spells.
	const float ClampedScale = FMath::Clamp(VisualScale, 0.5f, 4.f);
	MeshComp->SetRelativeScale3D(FVector(0.3f * ClampedScale));
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(14.f * ClampedScale);
	}

	if (!DynamicMaterial)
	{
		if (UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(0))
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (DynamicMaterial)
			{
				MeshComp->SetMaterial(0, DynamicMaterial);
			}
		}
	}

	if (DynamicMaterial)
	{
		// Set both common parameter names so a simple emissive material lights up whichever it exposes.
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	}
}
