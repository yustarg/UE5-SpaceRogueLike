// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Fast updates for smooth swarming in HORDE MODE
	UpdateFrequency = 0.1f; 
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

	// Get controlled pawn
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// Calculate direction to player
	FVector EnemyLocation = ControlledPawn->GetActorLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector Direction = (PlayerLocation - EnemyLocation).GetSafeNormal2D();

	// Directly add movement input (no pathfinding needed for space game)
	ControlledPawn->AddMovementInput(Direction, 1.0f);

	// Optionally rotate enemy to face player
	if (!Direction.IsNearlyZero())
	{
		FRotator TargetRotation = Direction.Rotation();
		ControlledPawn->SetActorRotation(TargetRotation);
	}
}
