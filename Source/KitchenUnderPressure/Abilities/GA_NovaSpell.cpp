// Copyright Epic Games, Inc. All Rights Reserved.

#include "GA_NovaSpell.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Core/KUPCombatStatics.h"
#include "Enemies/EnemyCharacter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

UGA_NovaSpell::UGA_NovaSpell()
{
	// A 360° burst hits everything around; slower than a single projectile.
	CooldownSeconds = 1.5f;
}

void UGA_NovaSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		float FinalDamage;
		FLinearColor FinalColor;
		TSubclassOf<UGameplayEffect> Status;
		float VisualScale;
		ResolvePayload(FinalDamage, FinalColor, Status, VisualScale);

		const float EffectiveRadius = Radius * VisualScale;

		AActor* Avatar = GetAvatarActorFromActorInfo();
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
		if (Avatar && SourceASC && World)
		{
			const FVector Center = Avatar->GetActorLocation();

			TArray<FOverlapResult> Overlaps;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(NovaSpell), /*bTraceComplex=*/false, Avatar);
			World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(EffectiveRadius), Params);

			// One application per victim, even if several of its components overlap. The friendly-fire
			// gate skips allies when the run has it disabled.
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
						ApplyDamageAndStatus(SourceASC, TargetASC, FinalDamage, Status);

						// Report the elemental hit so a second player's opposed element can react (GDD 8.1).
						if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Other))
						{
							Enemy->NotifyElementalHit(SourceASC, GetEquippedElement());
						}
					}
				}
			}

#if ENABLE_DRAW_DEBUG
			// Temporary readability for the burst radius/colour (server-side); replaced by FX in M8.
			DrawDebugSphere(World, Center, EffectiveRadius, 16, FinalColor.ToFColor(true), false, 0.4f, 0, 2.f);
#endif
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}
