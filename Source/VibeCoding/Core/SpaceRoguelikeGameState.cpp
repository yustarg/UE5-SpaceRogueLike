// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceRoguelikeGameState.h"
#include "Net/UnrealNetwork.h"

ASpaceRoguelikeGameState::ASpaceRoguelikeGameState()
{
	TotalKills = 0;
}

void ASpaceRoguelikeGameState::AddKill()
{
	TotalKills++;
	OnRep_TotalKills();
}

float ASpaceRoguelikeGameState::GetGameTime() const
{
	return GetServerWorldTimeSeconds();
}

void ASpaceRoguelikeGameState::OnRep_TotalKills()
{
	// Broadcast to UI
	OnKillCountChanged.Broadcast(TotalKills);
	UE_LOG(LogTemp, Log, TEXT("Total Kills: %d"), TotalKills);
}

void ASpaceRoguelikeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpaceRoguelikeGameState, TotalKills);
}
