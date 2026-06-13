// Copyright Epic Games, Inc. All Rights Reserved.

#include "GA_ProjectileSpell.h"
#include "Spells/SpellProjectile.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"

UGA_ProjectileSpell::UGA_ProjectileSpell()
{
	// Use the built-in projectile by default so no projectile Blueprint is required.
	ProjectileClass = ASpellProjectile::StaticClass();
}

void UGA_ProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
		return;
	}

	// ServerOnly ability activated by the server, so this branch always runs on the authority.
	if (HasAuthority(&ActivationInfo) && ProjectileClass)
	{
		float FinalDamage;
		FLinearColor FinalColor;
		TSubclassOf<UGameplayEffect> Status;
		float VisualScale;
		ResolvePayload(FinalDamage, FinalColor, Status, VisualScale);

		FVector MuzzleLocation;
		FVector MuzzleDirection;
		GetMuzzle(MuzzleLocation, MuzzleDirection);

		AActor* Avatar = GetAvatarActorFromActorInfo();
		if (UWorld* World = Avatar ? Avatar->GetWorld() : nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Avatar;
			SpawnParams.Instigator = Cast<APawn>(Avatar);
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn slightly ahead of the eyes so the projectile starts outside the caster's capsule.
			const FVector SpawnLocation = MuzzleLocation + MuzzleDirection * 100.f;
			ASpellProjectile* Projectile = World->SpawnActor<ASpellProjectile>(ProjectileClass, SpawnLocation, MuzzleDirection.Rotation(), SpawnParams);
			if (Projectile)
			{
				UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
				Projectile->InitProjectile(MuzzleDirection * ProjectileSpeed, FinalDamage, GetDamageEffectClass(), Status, SourceASC, FinalColor, VisualScale, GetEquippedElement());
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}
