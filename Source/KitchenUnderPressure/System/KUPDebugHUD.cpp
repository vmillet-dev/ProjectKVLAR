// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPDebugHUD.h"

#if !UE_BUILD_SHIPPING
#include "AbilitySystemComponent.h"
#include "AlchemistPlayerState.h"
#include "Attributes/AlchemistAttributeSet.h"
#include "Enemies/EnemyCharacter.h"
#include "KUPGameplayTags.h"
#include "Spells/SpellTypes.h"
#include "Spells/SpellRegistrySubsystem.h"
#include "Spells/ElementDefinition.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarKUPDebugHUD(
	TEXT("KUP.Debug.HUD"), 1,
	TEXT("Prototype debug overlay: enemy health bars + local player spell panel (1 = on, 0 = off)."));

namespace
{
	/** "Element.Fire" -> "Fire" (tags read better short on a debug overlay). */
	FString ShortTagName(const FGameplayTag& Tag)
	{
		const FString Full = Tag.GetTagName().ToString();
		FString Short;
		return Full.Split(TEXT("."), nullptr, &Short, ESearchCase::IgnoreCase, ESearchDir::FromEnd) ? Short : Full;
	}

	const TCHAR* RarityName(ESpellRarity Rarity)
	{
		switch (Rarity)
		{
		case ESpellRarity::Rare:      return TEXT("Rare");
		case ESpellRarity::Epic:      return TEXT("Epic");
		case ESpellRarity::Legendary: return TEXT("Legendary");
		default:                      return TEXT("Common");
		}
	}
}
#endif // !UE_BUILD_SHIPPING

void AKUPDebugHUD::DrawHUD()
{
	Super::DrawHUD();

#if !UE_BUILD_SHIPPING
	if (!Canvas || CVarKUPDebugHUD.GetValueOnGameThread() == 0)
	{
		return;
	}
	DrawEnemyHealthBars();
	DrawLocalPlayerPanel();
#endif
}

#if !UE_BUILD_SHIPPING

FString AKUPDebugHUD::DescribeSpell(const FSpellDefinition& Spell)
{
	if (!Spell.IsValid())
	{
		return TEXT("(vide)");
	}

	FString Out = FString::Printf(TEXT("%s %s [%s] x%.1f"),
		*ShortTagName(Spell.Element), *ShortTagName(Spell.Form), RarityName(Spell.Rarity), Spell.Power);
	for (const FGameplayTag& ModifierTag : Spell.Modifiers)
	{
		Out += TEXT(" +") + ShortTagName(ModifierTag);
	}
	return Out;
}

void AKUPDebugHUD::DrawEnemyHealthBars()
{
	const APawn* ViewPawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	UFont* Font = GEngine->GetSmallFont();

	for (TActorIterator<AEnemyCharacter> It(GetWorld()); It; ++It)
	{
		const AEnemyCharacter* Enemy = *It;
		UAbilitySystemComponent* ASC = Enemy ? Enemy->GetAbilitySystemComponent() : nullptr;
		if (!ASC || ASC->HasMatchingGameplayTag(KUPTags::State_Dead))
		{
			continue;
		}

		const float MaxHealth = ASC->GetNumericAttribute(UAlchemistAttributeSet::GetMaxHealthAttribute());
		if (MaxHealth <= 0.f)
		{
			continue;
		}
		const float Health = ASC->GetNumericAttribute(UAlchemistAttributeSet::GetHealthAttribute());

		if (ViewPawn && FVector::DistSquared(ViewPawn->GetActorLocation(), Enemy->GetActorLocation()) > FMath::Square(6000.f))
		{
			continue; // too far to matter; keeps the overlay readable in big rooms
		}

		const FVector WorldPos = Enemy->GetActorLocation() + FVector(0.f, 0.f, Enemy->GetSimpleCollisionHalfHeight() + 35.f);
		const FVector ScreenPos = Project(WorldPos);
		if (ScreenPos.Z <= 0.f)
		{
			continue; // behind the camera
		}

		const float Ratio = FMath::Clamp(Health / MaxHealth, 0.f, 1.f);
		const float BarWidth = 70.f;
		const float BarHeight = 7.f;
		const float X = ScreenPos.X - BarWidth * 0.5f;
		const float Y = ScreenPos.Y;

		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), X - 1.f, Y - 1.f, BarWidth + 2.f, BarHeight + 2.f);
		DrawRect(FLinearColor(1.f - Ratio, Ratio, 0.05f, 0.95f), X, Y, BarWidth * Ratio, BarHeight);
		DrawText(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth), FLinearColor::White, X, Y - 14.f, Font);
	}
}

void AKUPDebugHUD::DrawLocalPlayerPanel()
{
	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	const AAlchemistPlayerState* PS = Pawn ? Pawn->GetPlayerState<AAlchemistPlayerState>() : nullptr;
	if (!PS)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float X = 20.f;
	float Y = Canvas->SizeY - 90.f;

	if (const UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
	{
		const float Health = ASC->GetNumericAttribute(UAlchemistAttributeSet::GetHealthAttribute());
		const float MaxHealth = ASC->GetNumericAttribute(UAlchemistAttributeSet::GetMaxHealthAttribute());
		const float Ratio = (MaxHealth > 0.f) ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f;
		DrawText(FString::Printf(TEXT("PV %.0f / %.0f"), Health, MaxHealth), FLinearColor(1.f - Ratio, Ratio, 0.1f), X, Y, Font);
		Y += 20.f;
	}

	// One line per hand, tinted with the element colour (mirrors the in-world readability rule).
	USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this);
	auto ElementColor = [Registry](const FSpellDefinition& Spell) -> FLinearColor
	{
		if (Registry && Spell.IsValid())
		{
			if (const UElementDefinition* Element = Registry->FindElement(Spell.Element))
			{
				return Element->Color;
			}
		}
		return FLinearColor::White;
	};

	const FSpellDefinition& LeftSpell = PS->GetSpell(EHand::Left);
	DrawText(TEXT("Main gauche (clic G) : ") + DescribeSpell(LeftSpell), ElementColor(LeftSpell), X, Y, Font);
	Y += 20.f;

	const FSpellDefinition& RightSpell = PS->GetSpell(EHand::Right);
	DrawText(TEXT("Main droite (clic D) : ") + DescribeSpell(RightSpell), ElementColor(RightSpell), X, Y, Font);
}

#endif // !UE_BUILD_SHIPPING
