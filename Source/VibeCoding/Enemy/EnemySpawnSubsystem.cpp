// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawnSubsystem.h"
#include "EnemyBase.h"
#include "EnemyAIController.h"
#include "EnemyPoolSubsystem.h"
#include "SpaceShipAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

void UEnemySpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create HISM Component on a dummy actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transient; // Don't save this actor
	HISMContainerActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (HISMContainerActor)
	{
		HISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(HISMContainerActor, TEXT("EnemyHISM"));
		HISMComponent->RegisterComponent();
		HISMContainerActor->SetRootComponent(HISMComponent);
		
		// Optimization: Disable collision for HISM (we use Actor capsules for collision)
		HISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HISMComponent->CastShadow = false;
		HISMComponent->SetCanEverAffectNavigation(false);
	}
	
	// Initialize free list
	FreeHISMIndices.Empty();

	// Set default values - TRUE SURVIVOR MODE 🌊🌊🌊
	InitialSpawnInterval = 0.3f;      // Even faster start
	MinSpawnInterval = 0.05f;         // Extreme speed at high difficulty
	CurrentSpawnInterval = InitialSpawnInterval;
	MaxActiveEnemies = 1000;          // Massive horde (was 200)
	DifficultyAdjustInterval = 5.0f;  // Scale every 5 seconds (was 10s)
	SpawnIntervalMultiplier = 0.9f;   
	SpawnDistance = 2500.0f;          // Spawn a bit further out for visual scale
	DespawnDistance = 6000.0f;
	GameTime = 0.0f;

	// Start spawning
	UpdateSpawnTimer();

	UE_LOG(LogTemp, Warning, TEXT("EnemySpawnSubsystem Initialized - HISM SURVIVOR MODE!"));
}

void UEnemySpawnSubsystem::Deinitialize()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}

	// Cleanup HISM
	FreeHISMIndices.Empty();
	
	if (HISMComponent)
	{
		HISMComponent->ClearInstances();
		HISMComponent = nullptr;
	}
	
	if (HISMContainerActor)
	{
		HISMContainerActor->Destroy();
		HISMContainerActor = nullptr;
	}

	Super::Deinitialize();
}

void UEnemySpawnSubsystem::Tick(float DeltaTime)
{
	GameTime += DeltaTime;

	// Adjust difficulty every interval
	static float TimeSinceLastAdjustment = 0.0f;
	TimeSinceLastAdjustment += DeltaTime;

	if (TimeSinceLastAdjustment >= DifficultyAdjustInterval)
	{
		TimeSinceLastAdjustment = 0.0f;

		// Reduce spawn interval
		CurrentSpawnInterval *= SpawnIntervalMultiplier;
		CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval, MinSpawnInterval);

		// Update spawn timer
		UpdateSpawnTimer();
	}

	// Update HISM Instances
	if (HISMComponent)
	{
		for (AEnemyBase* Enemy : ActiveEnemies)
		{
		if (IsValid(Enemy) && Enemy->GetHISMInstanceIndex() != INDEX_NONE)
			{
				if (Enemy->IsHidden())
				{
					// Hide by scaling to zero
				HISMComponent->UpdateInstanceTransform(Enemy->GetHISMInstanceIndex(), FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector), true, false);
				}
				else
				{
				HISMComponent->UpdateInstanceTransform(Enemy->GetHISMInstanceIndex(), Enemy->GetActorTransform(), true, false);
				}
			}
		}
		// Batch send to GPU
		HISMComponent->MarkRenderStateDirty();
	}

	// Cleanup distant enemies (less frequent for performance)
	static float CleanupTimer = 0.0f;
	CleanupTimer += DeltaTime;
	if (CleanupTimer >= 1.0f)
	{
		CleanupTimer = 0.0f;
		CleanupDistantEnemies();
	}
}

void UEnemySpawnSubsystem::SetEnemyMesh(UStaticMesh* Mesh)
{
	if (HISMComponent && Mesh)
	{
		HISMComponent->SetStaticMesh(Mesh);
	}
}


TStatId UEnemySpawnSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEnemySpawnSubsystem, STATGROUP_Tickables);
}

void UEnemySpawnSubsystem::OnEnemyDied(AEnemyBase* Enemy)
{
	if (Enemy)
	{
		ActiveEnemies.Remove(Enemy);

		// Release HISM index back to free list
		int32 HISMIndex = Enemy->GetHISMInstanceIndex();
		if (HISMIndex != INDEX_NONE)
		{
			ReleaseHISMIndex(HISMIndex);
			Enemy->SetHISMInstanceIndex(INDEX_NONE);
		}
	}
}

void UEnemySpawnSubsystem::SpawnEnemy()
{
	if (!EnemyClass)
	{
		return;
	}

	// Auto-setup HISM mesh from enemy class CDO (only once)
	if (HISMComponent && !HISMComponent->GetStaticMesh())
	{
		// Try to get mesh from EnemyClass CDO
		if (const AEnemyBase* EnemyCDO = EnemyClass->GetDefaultObject<AEnemyBase>())
		{
			UStaticMesh* Mesh = EnemyCDO->GetEnemyMesh();
			if (Mesh)
			{
				HISMComponent->SetStaticMesh(Mesh);
				UE_LOG(LogTemp, Warning, TEXT("HISM: Auto-set mesh from EnemyClass CDO: %s"), *Mesh->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("HISM: EnemyClass has no EnemyMesh set! Enemies will be invisible. Set EnemyMesh in the Blueprint."));
			}
		}
	}

	// Spawn massive batches based on game time
	// Start with 5, increase by 1 every 30 seconds
	int32 BaseBatchSize = 5;
	int32 TimeBonus = FMath::FloorToInt(GameTime / 30.0f);
	int32 EnemiesToSpawn = BaseBatchSize + TimeBonus;
	
	// Limit to reasonable batch for stability
	EnemiesToSpawn = FMath::Min(EnemiesToSpawn, 20);

	for (int32 i = 0; i < EnemiesToSpawn; i++)
	{
		if (ActiveEnemies.Num() >= MaxActiveEnemies)
		{
			return;
		}

		FVector SpawnLocation = GetRandomSpawnLocation();

		UEnemyPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UEnemyPoolSubsystem>();
		AEnemyBase* Enemy = nullptr;

		if (PoolSubsystem)
		{
			Enemy = PoolSubsystem->AcquireEnemy(EnemyClass, SpawnLocation);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		}

		if (Enemy)
		{
			ApplyEnemyScaling(Enemy, GameTime);
			ActiveEnemies.Add(Enemy);

			// HISM Integration - always acquire new index (old one was released on death)
			if (HISMComponent && HISMComponent->GetStaticMesh())
			{
				// Ensure enemy has no stale index (should be INDEX_NONE from pool)
				if (Enemy->GetHISMInstanceIndex() != INDEX_NONE)
				{
					UE_LOG(LogTemp, Warning, TEXT("Enemy had stale HISM index %d, releasing it"), Enemy->GetHISMInstanceIndex());
					ReleaseHISMIndex(Enemy->GetHISMInstanceIndex());
				}
				
				// Acquire index from free list or create new
				int32 NewIndex = AcquireHISMIndex(Enemy->GetActorTransform());
				Enemy->SetHISMInstanceIndex(NewIndex);
			}
		}
	}
}

FVector UEnemySpawnSubsystem::GetRandomSpawnLocation() const
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player)
	{
		return FVector::ZeroVector;
	}

	FVector PlayerLocation = Player->GetActorLocation();

	// Random angle around player (360 degrees)
	float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	float RadAngle = FMath::DegreesToRadians(RandomAngle);

	// Random distance for layered waves (between SpawnDistance and SpawnDistance * 1.5)
	float RandomDistance = FMath::RandRange(SpawnDistance * 0.8f, SpawnDistance * 1.2f);

	// Calculate offset
	FVector Offset = FVector(
		FMath::Cos(RadAngle) * RandomDistance,
		FMath::Sin(RadAngle) * RandomDistance,
		0.0f
	);

	return PlayerLocation + Offset;
}

void UEnemySpawnSubsystem::ApplyEnemyScaling(AEnemyBase* Enemy, float GameTimeSeconds) const
{
	if (!Enemy)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	USpaceShipAttributeSet* Attributes = const_cast<USpaceShipAttributeSet*>(
		ASC->GetSet<USpaceShipAttributeSet>()
	);

	if (Attributes)
	{
		// Simple scaling: +10% health per minute
		float HealthMultiplier = 1.0f + (GameTimeSeconds / 60.0f) * 0.1f;
		float BaseHealth = 100.0f;
		float ScaledHealth = BaseHealth * HealthMultiplier;

		Attributes->SetMaxHealth(ScaledHealth);
		Attributes->SetHealth(ScaledHealth);

		UE_LOG(LogTemp, Log, TEXT("Enemy scaled: Health = %.2f (Multiplier: %.2f)"), ScaledHealth, HealthMultiplier);
	}
}

void UEnemySpawnSubsystem::CleanupDistantEnemies()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player)
	{
		return;
	}

	FVector PlayerLocation = Player->GetActorLocation();

	// Check all active enemies
	for (int32 i = ActiveEnemies.Num() - 1; i >= 0; i--)
	{
		AEnemyBase* Enemy = ActiveEnemies[i];
		
		// Remove invalid enemies
		if (!IsValid(Enemy))
		{
			ActiveEnemies.RemoveAt(i);
			continue;
		}

		// Check distance
		float Distance = FVector::Dist(Enemy->GetActorLocation(), PlayerLocation);
		if (Distance > DespawnDistance)
		{
			UE_LOG(LogTemp, Log, TEXT("Despawning distant enemy (%.2f units away)"), Distance);
			Enemy->Deactivate();
			// No need to remove from ActiveEnemies here, Deactivate() calls OnEnemyDied() which handles it
		}
	}
}

void UEnemySpawnSubsystem::UpdateSpawnTimer()
{
	if (!GetWorld())
	{
		return;
	}

	// Clear existing timer
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	// Set new timer
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&UEnemySpawnSubsystem::SpawnEnemy,
		CurrentSpawnInterval,
		true // Loop
	);
}

int32 UEnemySpawnSubsystem::AcquireHISMIndex(const FTransform& Transform)
{
	if (!HISMComponent)
	{
		return INDEX_NONE;
	}

	// Try to reuse from free list first
	if (FreeHISMIndices.Num() > 0)
	{
		int32 ReuseIndex = FreeHISMIndices.Pop(false);
		
		// Validate index is still in range
		if (ReuseIndex >= 0 && ReuseIndex < HISMComponent->GetInstanceCount())
		{
			// Update transform to show the instance
			HISMComponent->UpdateInstanceTransform(ReuseIndex, Transform, true, true);
			return ReuseIndex;
		}
		// Invalid index, skip it and create new
	}

	// No free index available, create new instance
	return HISMComponent->AddInstance(Transform, true);
}

void UEnemySpawnSubsystem::ReleaseHISMIndex(int32 Index)
{
	if (!HISMComponent || Index == INDEX_NONE)
	{
		return;
	}

	// Validate index is in range
	if (Index < 0 || Index >= HISMComponent->GetInstanceCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to release invalid HISM index %d (count: %d)"), Index, HISMComponent->GetInstanceCount());
		return;
	}

	// Hide the instance by scaling to zero
	HISMComponent->UpdateInstanceTransform(Index, FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector), true, true);

	// Add to free list for reuse
	FreeHISMIndices.Add(Index);
}
