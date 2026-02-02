// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyPoolSubsystem.generated.h"

class AEnemyBase;

USTRUCT()
struct FEnemyPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AEnemyBase*> InactiveEnemies;
};

/**
 * Subsystem to manage enemy pooling
 */
UCLASS()
class VIBECODING_API UEnemyPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Get an enemy from the pool or spawn a new one */
	UFUNCTION(BlueprintCallable, Category = "Combat|Pooling")
	AEnemyBase* AcquireEnemy(TSubclassOf<AEnemyBase> EnemyClass, FVector Location);

	/** Add an enemy back to the inactive pool */
	void ReturnToPool(AEnemyBase* Enemy);

private:
	UPROPERTY()
	TMap<TSubclassOf<AEnemyBase>, FEnemyPool> PoolMap;
};
