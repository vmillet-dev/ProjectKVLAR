// Copyright Epic Games, Inc. All Rights Reserved.

#include "GA_ComboReaction.h"
#include "Spells/SpellCasterComponent.h"
#include "Spells/ComboBurst.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "System/KUPCombatStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

UGA_ComboReaction::UGA_ComboReaction()
{
	// Use the self-contained burst by default so combos have a visual without any authored asset.
	BurstClass = AComboBurst::StaticClass();

	// The caster component already debounces combos; no extra fire-rate guard on top.
	CooldownSeconds = 0.f;
}

void UGA_ComboReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		const USpellCasterComponent* Caster = Cast<USpellCasterComponent>(GetCurrentSourceObject());
		AActor* Avatar = GetAvatarActorFromActorInfo();
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;

		if (Caster && SourceASC && World)
		{
			const FPendingCombo& Combo = Caster->GetPendingCombo();

			TArray<FOverlapResult> Overlaps;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(ComboReaction), /*bTraceComplex=*/false, Avatar);
			World->OverlapMultiByChannel(Overlaps, Combo.Location, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Combo.Radius), Params);

			TSet<UAbilitySystemComponent*> AlreadyHit;
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* Other = Overlap.GetActor();
				if (!Other || Other == Avatar)
				{
					continue;
				}
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
				{
					if (!AlreadyHit.Contains(TargetASC) && UKUPCombatStatics::CanDamage(SourceASC, TargetASC))
					{
						AlreadyHit.Add(TargetASC);
						ApplyDamageAndStatus(SourceASC, TargetASC, Combo.Damage, Combo.StatusEffect);
					}
				}
			}

			if (BurstClass)
			{
				if (AComboBurst* Burst = World->SpawnActor<AComboBurst>(BurstClass, Combo.Location, FRotator::ZeroRotator))
				{
					Burst->InitBurst(Combo.Color, Combo.Radius);
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}
