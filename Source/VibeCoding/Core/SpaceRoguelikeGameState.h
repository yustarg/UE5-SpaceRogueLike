// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpaceRoguelikeGameState.generated.h"

// Delegate for kill count changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32, NewKillCount);

/**
 * Game State for Space Roguelike
 * Stores replicated game state (kill count, game time, etc.)
 */
UCLASS()
class VIBECODING_API ASpaceRoguelikeGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASpaceRoguelikeGameState();

protected:
	/** Total number of enemies killed */
	UPROPERTY(ReplicatedUsing = OnRep_TotalKills, BlueprintReadOnly, Category = "Game Stats")
	int32 TotalKills;

public:
	/** Add a kill to the counter */
	UFUNCTION(BlueprintCallable, Category = "Game Stats")
	void AddKill();

	/** Get total kills */
	UFUNCTION(BlueprintPure, Category = "Game Stats")
	int32 GetTotalKills() const { return TotalKills; }

	/** Get game time in seconds */
	UFUNCTION(BlueprintPure, Category = "Game Stats")
	float GetGameTime() const;

	/** Delegate broadcasted when kill count changes */
	UPROPERTY(BlueprintAssignable, Category = "Game Stats")
	FOnKillCountChanged OnKillCountChanged;

protected:
	UFUNCTION()
	void OnRep_TotalKills();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
