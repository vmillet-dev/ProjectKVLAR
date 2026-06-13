// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboBurst.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

AComboBurst::AComboBurst()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.6f; // a touch longer than the expand so it fully forms before despawning

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCastShadow(false);

	// Engine primitives so the combo has a visual with no authored content. The sphere is 50 uu radius
	// at scale 1; BasicShapeMaterial exposes a "Color" parameter we drive per burst.
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

	MeshComp->SetRelativeScale3D(FVector::ZeroVector);
}

void AComboBurst::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AComboBurst, Color);
	DOREPLIFETIME(AComboBurst, TargetRadius);
}

void AComboBurst::InitBurst(const FLinearColor& InColor, float InRadius)
{
	Color = InColor;
	TargetRadius = InRadius;
	ApplyVisual();
}

void AComboBurst::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisual();
}

void AComboBurst::OnRep_Visual()
{
	ApplyVisual();
}

void AComboBurst::ApplyVisual()
{
	if (!MeshComp)
	{
		return;
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
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	}
}

void AComboBurst::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	const float Alpha = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	const float Scale = (TargetRadius / 50.f) * Alpha; // 50 uu = engine sphere radius at scale 1
	if (MeshComp)
	{
		MeshComp->SetRelativeScale3D(FVector(Scale));
	}
}
