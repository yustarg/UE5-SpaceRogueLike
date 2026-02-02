// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawnSubsystem.h"
#include "EnemyBase.h"
#include "EnemyAIController.h"
#include "SpaceShipAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

void UEnemySpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

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

	UE_LOG(LogTemp, Warning, TEXT("EnemySpawnSubsystem Initialized - SURVIVOR MODE ACTIVE! MaxEnemies: %d"), MaxActiveEnemies);
}

void UEnemySpawnSubsystem::Deinitialize()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
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

	// Cleanup distant enemies (less frequent for performance)
	static float CleanupTimer = 0.0f;
	CleanupTimer += DeltaTime;
	if (CleanupTimer >= 1.0f)
	{
		CleanupTimer = 0.0f;
		CleanupDistantEnemies();
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
	}
}

void UEnemySpawnSubsystem::SpawnEnemy()
{
	if (!EnemyClass)
	{
		return;
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

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(
			EnemyClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Enemy)
		{
			ApplyEnemyScaling(Enemy, GameTime);
			ActiveEnemies.Add(Enemy);
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
			Enemy->Destroy();
			ActiveEnemies.RemoveAt(i);
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
