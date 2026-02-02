// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyPoolSubsystem.h"
#include "EnemyBase.h"

AEnemyBase* UEnemyPoolSubsystem::AcquireEnemy(TSubclassOf<AEnemyBase> EnemyClass, FVector Location)
{
	if (!EnemyClass) return nullptr;

	FEnemyPool& Pool = PoolMap.FindOrAdd(EnemyClass);
	AEnemyBase* Enemy = nullptr;

	// Try to find an inactive one
	for (int32 i = Pool.InactiveEnemies.Num() - 1; i >= 0; --i)
	{
		AEnemyBase* Potential = Pool.InactiveEnemies[i];
		if (IsValid(Potential) && Potential->IsHidden())
		{
			Enemy = Potential;
			Pool.InactiveEnemies.RemoveAt(i);
			break;
		}
		else if (!IsValid(Potential))
		{
			Pool.InactiveEnemies.RemoveAt(i);
		}
	}

	// If none found, spawn a new one
	if (!Enemy)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, Location, FRotator::ZeroRotator, SpawnParams);
	}

	if (Enemy)
	{
		Enemy->Activate(Location);
	}

	return Enemy;
}

void UEnemyPoolSubsystem::ReturnToPool(AEnemyBase* Enemy)
{
	if (IsValid(Enemy))
	{
		FEnemyPool& Pool = PoolMap.FindOrAdd(Enemy->GetClass());
		Pool.InactiveEnemies.AddUnique(Enemy);
	}
}
