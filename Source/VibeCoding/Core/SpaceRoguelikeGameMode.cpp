// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceRoguelikeGameMode.h"
#include "SpaceRoguelikeGameState.h"
#include "SpaceRoguelikePlayerController.h"
#include "SpaceShip.h"
#include "Kismet/GameplayStatics.h"

ASpaceRoguelikeGameMode::ASpaceRoguelikeGameMode()
{
	// Set default classes
	GameStateClass = ASpaceRoguelikeGameState::StaticClass();
	PlayerControllerClass = ASpaceRoguelikePlayerController::StaticClass();
	DefaultPawnClass = ASpaceShip::StaticClass();
}

void ASpaceRoguelikeGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("SpaceRoguelikeGameMode: Game Started"));
}

void ASpaceRoguelikeGameMode::OnPlayerDeath(AController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("SpaceRoguelikeGameMode: Player Died"));

	// Disable input
	ASpaceRoguelikePlayerController* PC = Cast<ASpaceRoguelikePlayerController>(PlayerController);
	if (PC)
	{
		PC->ShowGameOverUI();
	}
}

void ASpaceRoguelikeGameMode::RestartGame()
{
	// Reload current level
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
