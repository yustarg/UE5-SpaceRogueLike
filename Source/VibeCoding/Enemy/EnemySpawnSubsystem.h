// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemySpawnSubsystem.generated.h"

class AEnemyBase;
class UInstancedStaticMeshComponent;

/**
 * Enemy Spawn Subsystem
 * Manages enemy spawning with ISM (Instanced Static Mesh) rendering optimization
 */
UCLASS()
class VIBECODING_API UEnemySpawnSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;

	/** Set the mesh used for ISM rendering */
	UFUNCTION(BlueprintCallable, Category = "Spawning|Optimization")
	void SetEnemyMesh(UStaticMesh* Mesh);

	/** Called when an enemy dies */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void OnEnemyDied(AEnemyBase* Enemy);

protected:
	//=============================================================================
	// ISM Rendering
	//=============================================================================
	
	/** ISM Component for crowd rendering */
	UPROPERTY()
	UInstancedStaticMeshComponent* ISMComponent;

	/** Container actor for ISM component */
	UPROPERTY()
	AActor* ISMContainerActor;

	/** Free list for recycling ISM instance indices */
	TArray<int32> FreeISMIndices;

	/** Whether ISM has been initialized */
	bool bISMInitialized;

	/** Scale multiplier for ISM instances */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawning|Optimization")
	float ISMScaleMultiplier;

	/** Initialize ISM component (called on first tick) */
	void InitializeISM();

	/** Acquire an ISM instance index */
	int32 AcquireISMIndex(const FTransform& Transform);

	/** Release an ISM instance index back to free list */
	void ReleaseISMIndex(int32 Index);

	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return !IsTemplate(); }

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

	/** Distance from player to spawn enemies */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float SpawnDistance;

	/** Distance from player to despawn enemies */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float DespawnDistance;

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
	
	void SpawnEnemy();
	FVector GetRandomSpawnLocation() const;
	void ApplyEnemyScaling(AEnemyBase* Enemy, float GameTimeSeconds) const;
	void CleanupDistantEnemies();
	void UpdateSpawnTimer();
};
