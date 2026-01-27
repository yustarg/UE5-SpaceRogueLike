// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	UpdateFrequency = 0.5f; // Update AI every 0.5 seconds (performance)
	TimeSinceLastUpdate = 0.0f;
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	// Cache player pawn
	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastUpdate += DeltaTime;

	// Update AI at specified frequency
	if (TimeSinceLastUpdate >= UpdateFrequency)
	{
		TimeSinceLastUpdate = 0.0f;
		ChasePlayer();
	}
}

void AEnemyAIController::ChasePlayer()
{
	if (!PlayerPawn)
	{
		// Try to find player again
		PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		if (!PlayerPawn)
		{
			return;
		}
	}

	// Move towards player
	// MVP: Simple move to location (no pathfinding for space game)
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	MoveToLocation(PlayerLocation, 50.0f); // Accept radius of 50 units
}
