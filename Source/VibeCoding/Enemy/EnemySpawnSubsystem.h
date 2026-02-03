// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemySpawnSubsystem.generated.h"

class AEnemyBase;
class UHierarchicalInstancedStaticMeshComponent;

/**
 * Enemy Spawn Subsystem
 * Manages enemy spawning with HISM rendering optimization
 */
UCLASS()
class VIBECODING_API UEnemySpawnSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// ... (保留 Initialize/Deinitialize)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ... (保留 Tick 等)
	virtual void Tick(float DeltaTime) override;

	/** Set the mesh used for HISM (called from BP or World Settings) */
	UFUNCTION(BlueprintCallable, Category = "Spawning|Optimization")
	void SetEnemyMesh(UStaticMesh* Mesh);

	/** Called when an enemy dies (remove from active list) */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void OnEnemyDied(AEnemyBase* Enemy);

protected:
	/** HISM Component for massive crowd rendering */
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* HISMComponent;

	/** Container actor for HISM component (stored for cleanup) */
	UPROPERTY()
	AActor* HISMContainerActor;

	/** Free list for recycling HISM instance indices */
	TArray<int32> FreeHISMIndices;

	/** Acquire a HISM instance index (from free list or create new) */
	int32 AcquireHISMIndex(const FTransform& Transform);

	/** Release a HISM instance index back to free list */
	void ReleaseHISMIndex(int32 Index);

	/** The mesh to use for all enemies */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning|Optimization")
	UStaticMesh* EnemyStaticMesh;

	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return !IsTemplate(); }

protected:
	//=============================================================================
	// Spawning
	//=============================================================================
	
	/** Enemy class to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AEnemyBase> EnemyClass;

	/** Current spawn interval (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Spawning")
	float CurrentSpawnInterval;

	/** Initial spawn interval */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float InitialSpawnInterval;

	/** Minimum spawn interval (difficulty cap) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float MinSpawnInterval;

	/** Timer handle for spawning */
	FTimerHandle SpawnTimerHandle;

	/** Active enemies */
	UPROPERTY()
	TArray<AEnemyBase*> ActiveEnemies;

	/** Maximum active enemies */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	int32 MaxActiveEnemies;

	//=============================================================================
	// Difficulty
	//=============================================================================
	
	/** Game time in seconds */
	float GameTime;

	/** Difficulty adjustment interval (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Difficulty")
	float DifficultyAdjustInterval;

	/** Spawn interval multiplier per adjustment */
	UPROPERTY(EditDefaultsOnly, Category = "Difficulty")
	float SpawnIntervalMultiplier;

	//=============================================================================
	// Spawning Functions
	//=============================================================================
	
	/** Spawn a single enemy */
	UFUNCTION()
	void SpawnEnemy();

	/** Get random spawn location around player */
	FVector GetRandomSpawnLocation() const;

	/** Apply enemy scaling based on game time */
	void ApplyEnemyScaling(AEnemyBase* Enemy, float GameTimeSeconds) const;

	/** Clean up distant enemies */
	void CleanupDistantEnemies();

	/** Update spawn timer with new interval */
	void UpdateSpawnTimer();

	/** Distance from player to spawn enemies */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float SpawnDistance;

	/** Distance from player to despawn enemies */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float DespawnDistance;
};
