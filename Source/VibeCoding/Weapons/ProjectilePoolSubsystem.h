// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolSubsystem.generated.h"

class AProjectile;

USTRUCT()
struct FProjectilePool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AProjectile*> InactiveProjectiles;
};

/**
 * Subsystem to manage projectile pooling
 */
UCLASS()
class VIBECODING_API UProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Get a projectile from the pool or spawn a new one */
	UFUNCTION(BlueprintCallable, Category = "Combat|Pooling")
	AProjectile* AcquireProjectile(TSubclassOf<AProjectile> ProjectileClass, FVector Location, FRotator Rotation, AActor* InOwner);

	/** Add a projectile back to the inactive pool */
	void ReturnToPool(AProjectile* Projectile);

private:
	UPROPERTY()
	TMap<TSubclassOf<AProjectile>, FProjectilePool> PoolMap;
};
