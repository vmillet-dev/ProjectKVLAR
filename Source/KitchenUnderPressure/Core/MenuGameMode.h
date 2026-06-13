// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuGameMode.generated.h"

/**
 * Game mode for the main-menu map. Spawns no pawn; it only assigns the menu player
 * controller that builds the main-menu UI.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMenuGameMode();
};
