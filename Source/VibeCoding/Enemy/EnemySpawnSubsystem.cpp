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

	// Set default values
	InitialSpawnInterval = 2.0f;
	MinSpawnInterval = 0.5f;
	CurrentSpawnInterval = InitialSpawnInterval;
	MaxActiveEnemies = 50;
	DifficultyAdjustInterval = 30.0f;
	SpawnIntervalMultiplier = 0.9f;
	SpawnDistance = 2000.0f;
	DespawnDistance = 5000.0f;
	GameTime = 0.0f;

	// Start spawning
	UpdateSpawnTimer();

	UE_LOG(LogTemp, Log, TEXT("EnemySpawnSubsystem Initialized"));
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

		// Reduce spawn interval (increase difficulty)
		CurrentSpawnInterval *= SpawnIntervalMultiplier;
		CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval, MinSpawnInterval);

		// Update spawn timer
		UpdateSpawnTimer();

		UE_LOG(LogTemp, Warning, TEXT("Difficulty Increased! New spawn interval: %.2f seconds"), CurrentSpawnInterval);
	}

	// Cleanup distant enemies
	CleanupDistantEnemies();
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
	// Check if we've reached the max
	if (ActiveEnemies.Num() >= MaxActiveEnemies)
	{
		return;
	}

	// Get enemy class (in real game, load from config)
	if (!EnemyClass)
	{
		// Try to load default enemy class
		// For MVP, we'll just log a warning
		UE_LOG(LogTemp, Warning, TEXT("EnemySpawnSubsystem: No EnemyClass set! Set it in Blueprint or World Settings."));
		return;
	}

	// Get spawn location
	FVector SpawnLocation = GetRandomSpawnLocation();

	// Spawn enemy
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
		// Apply enemy scaling
		ApplyEnemyScaling(Enemy, GameTime);

		// Set AI controller
		Enemy->AIControllerClass = AEnemyAIController::StaticClass();
		Enemy->SpawnDefaultController();

		// Add to active list
		ActiveEnemies.Add(Enemy);

		UE_LOG(LogTemp, Log, TEXT("Enemy spawned at %s"), *SpawnLocation.ToString());
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

	// Random angle around player
	float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	float RadAngle = FMath::DegreesToRadians(RandomAngle);

	// Calculate offset
	FVector Offset = FVector(
		FMath::Cos(RadAngle) * SpawnDistance,
		FMath::Sin(RadAngle) * SpawnDistance,
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
