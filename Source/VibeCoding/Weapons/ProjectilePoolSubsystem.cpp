// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectilePoolSubsystem.h"
#include "Projectile.h"

AProjectile* UProjectilePoolSubsystem::AcquireProjectile(TSubclassOf<AProjectile> ProjectileClass, FVector Location, FRotator Rotation, AActor* InOwner)
{
	if (!ProjectileClass) return nullptr;

	FProjectilePool& Pool = PoolMap.FindOrAdd(ProjectileClass);
	AProjectile* Projectile = nullptr;

	// Try to find an inactive one
	for (int32 i = Pool.InactiveProjectiles.Num() - 1; i >= 0; --i)
	{
		AProjectile* Potential = Pool.InactiveProjectiles[i];
		if (IsValid(Potential) && Potential->IsHidden())
		{
			Projectile = Potential;
			Pool.InactiveProjectiles.RemoveAt(i);
			break;
		}
		else if (!IsValid(Potential))
		{
			Pool.InactiveProjectiles.RemoveAt(i);
		}
	}

	// If none found, spawn a new one
	if (!Projectile)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
		
		UE_LOG(LogTemp, Log, TEXT("Pool Empty: Spawned new projectile. Pool size for %s: %d"), *ProjectileClass->GetName(), Pool.InactiveProjectiles.Num());
	}

	if (Projectile)
	{
		Projectile->Activate(Location, Rotation, InOwner);
	}

	return Projectile;
}

void UProjectilePoolSubsystem::ReturnToPool(AProjectile* Projectile)
{
	if (IsValid(Projectile))
	{
		FProjectilePool& Pool = PoolMap.FindOrAdd(Projectile->GetClass());
		Pool.InactiveProjectiles.AddUnique(Projectile);
	}
}
