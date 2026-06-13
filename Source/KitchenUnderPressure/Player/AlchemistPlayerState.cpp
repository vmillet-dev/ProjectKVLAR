// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlchemistPlayerState.h"
#include "Abilities/AlchemistAbilitySystemComponent.h"
#include "Attributes/AlchemistAttributeSet.h"
#include "Net/UnrealNetwork.h"

AAlchemistPlayerState::AAlchemistPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAlchemistAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Created on the same actor as the ASC, so it is auto-registered as a spawned attribute set.
	AttributeSet = CreateDefaultSubobject<UAlchemistAttributeSet>(TEXT("AttributeSet"));

	// PlayerState replicates slowly by default; raise it so health/cooldown UI stays responsive.
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AAlchemistPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAlchemistPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlchemistPlayerState, LeftSpell);
	DOREPLIFETIME(AAlchemistPlayerState, RightSpell);
}

void AAlchemistPlayerState::SetSpell(EHand Hand, const FSpellDefinition& Spell)
{
	if (Hand == EHand::Left)
	{
		LeftSpell = Spell;
	}
	else
	{
		RightSpell = Spell;
	}

	// Server/listen host path; remote clients get the same broadcast through the OnReps.
	OnHandSpellChanged.Broadcast(Hand);
}

void AAlchemistPlayerState::OnRep_LeftSpell()
{
	OnHandSpellChanged.Broadcast(EHand::Left);
}

void AAlchemistPlayerState::OnRep_RightSpell()
{
	OnHandSpellChanged.Broadcast(EHand::Right);
}
