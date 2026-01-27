// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpaceRoguelikeGameMode.generated.h"

/**
 * Game Mode for Space Roguelike
 * Handles game rules, player death, and game flow
 */
UCLASS()
class VIBECODING_API ASpaceRoguelikeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASpaceRoguelikeGameMode();

protected:
	virtual void BeginPlay() override;

public:
	/** Called when player dies */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnPlayerDeath(AController* PlayerController);

	/** Restart the game */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestartGame();
};
