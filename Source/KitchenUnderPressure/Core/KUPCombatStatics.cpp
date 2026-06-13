// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPCombatStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AlchemistGameState.h"
#include "KUPGameplayTags.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

bool UKUPCombatStatics::IsPlayerASC(const UAbilitySystemComponent* ASC)
{
	// Player ability systems are owned by the (Alchemist) PlayerState; enemy ones by the enemy pawn.
	return ASC && Cast<APlayerState>(ASC->GetOwnerActor()) != nullptr;
}

bool UKUPCombatStatics::CanDamage(const UAbilitySystemComponent* SourceASC, const UAbilitySystemComponent* TargetASC)
{
	if (!SourceASC || !TargetASC)
	{
		return false;
	}

	if (IsPlayerASC(SourceASC) && IsPlayerASC(TargetASC))
	{
		const UWorld* World = TargetASC->GetWorld();
		const AAlchemistGameState* GameState = World ? World->GetGameState<AAlchemistGameState>() : nullptr;
		return GameState && GameState->bFriendlyFire;
	}

	// Player vs enemy (either direction) is always allowed.
	return true;
}

void UKUPCombatStatics::ApplyAreaDamage(UWorld* World, UAbilitySystemComponent* SourceASC, const FVector& Center, float Radius, float DamageAmount, TSubclassOf<UGameplayEffect> DamageEffect, TSubclassOf<UGameplayEffect> StatusEffect)
{
	if (!World || !SourceASC || !DamageEffect || Radius <= 0.f)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KUPAreaDamage), /*bTraceComplex=*/false);
	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

	// One application per victim, even if several of its components overlap.
	TSet<UAbilitySystemComponent*> AlreadyHit;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Other = Overlap.GetActor();
		if (!Other)
		{
			continue;
		}
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other);
		if (!TargetASC || AlreadyHit.Contains(TargetASC) || !CanDamage(SourceASC, TargetASC))
		{
			continue;
		}
		AlreadyHit.Add(TargetASC);

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1.f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(KUPTags::Data_Damage, DamageAmount);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
		}

		if (StatusEffect)
		{
			const FGameplayEffectSpecHandle StatusSpec = SourceASC->MakeOutgoingSpec(StatusEffect, 1.f, Context);
			if (StatusSpec.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*StatusSpec.Data.Get(), TargetASC);
			}
		}
	}
}
