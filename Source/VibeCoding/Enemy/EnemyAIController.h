// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * AI Controller for Enemy
 * Simple chase behavior - move towards player
 */
UCLASS()
class VIBECODING_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	/** Update frequency (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float UpdateFrequency;

	/** Time since last update */
	float TimeSinceLastUpdate;

	/** Chase the player */
	void ChasePlayer();

	/** Cached player pawn */
	UPROPERTY()
	APawn* PlayerPawn;
};
