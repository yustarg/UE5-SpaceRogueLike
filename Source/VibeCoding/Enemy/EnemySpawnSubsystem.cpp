// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawnSubsystem.h"
#include "EnemyBase.h"
#include "EnemyAIController.h"
#include "EnemyPoolSubsystem.h"
#include "SpaceShipAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/InstancedStaticMeshComponent.h"

void UEnemySpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Only initialize in game worlds
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	// Initialize variables (ISM will be created on first tick)
	ISMComponent = nullptr;
	ISMContainerActor = nullptr;
	bISMInitialized = false;
	FreeISMIndices.Empty();

	// Set default values
	InitialSpawnInterval = 0.3f;
	MinSpawnInterval = 0.05f;
	CurrentSpawnInterval = InitialSpawnInterval;
	MaxActiveEnemies = 1000;
	DifficultyAdjustInterval = 5.0f;
	SpawnIntervalMultiplier = 0.9f;
	SpawnDistance = 2500.0f;
	DespawnDistance = 6000.0f;
	GameTime = 0.0f;
	ISMScaleMultiplier = 0.1f;  // Scale down mesh to 1/10

	// Start spawning
	UpdateSpawnTimer();

	UE_LOG(LogTemp, Log, TEXT("EnemySpawnSubsystem Initialized"));
}

void UEnemySpawnSubsystem::InitializeISM()
{
	if (bISMInitialized)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Create ISM Component on a container actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transient;
	ISMContainerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (ISMContainerActor)
	{
		ISMComponent = NewObject<UInstancedStaticMeshComponent>(ISMContainerActor, TEXT("EnemyISM"));
		
		if (ISMComponent)
		{
			ISMContainerActor->SetRootComponent(ISMComponent);
			
			ISMComponent->SetMobility(EComponentMobility::Movable);
			ISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ISMComponent->CastShadow = false;
			ISMComponent->SetCanEverAffectNavigation(false);
			ISMComponent->NumCustomDataFloats = 0;
			ISMComponent->SetCullDistances(0, 0);
			
			ISMComponent->RegisterComponent();
			ISMContainerActor->AddInstanceComponent(ISMComponent);
			
			ISMComponent->SetVisibility(true);
			ISMComponent->SetHiddenInGame(false);
			
			bISMInitialized = true;
			UE_LOG(LogTemp, Log, TEXT("ISM Component initialized"));
		}
	}
}

void UEnemySpawnSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}

	FreeISMIndices.Empty();
	
	if (ISMComponent)
	{
		ISMComponent->ClearInstances();
		ISMComponent = nullptr;
	}
	
	if (ISMContainerActor)
	{
		ISMContainerActor->Destroy();
		ISMContainerActor = nullptr;
	}

	Super::Deinitialize();
}

void UEnemySpawnSubsystem::Tick(float DeltaTime)
{
	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	// Delayed ISM initialization (first tick)
	if (!bISMInitialized)
	{
		InitializeISM();
	}

	if (!ISMComponent)
	{
		return;
	}

	GameTime += DeltaTime;

	// Adjust difficulty
	static float TimeSinceLastAdjustment = 0.0f;
	TimeSinceLastAdjustment += DeltaTime;

	if (TimeSinceLastAdjustment >= DifficultyAdjustInterval)
	{
		TimeSinceLastAdjustment = 0.0f;
		CurrentSpawnInterval *= SpawnIntervalMultiplier;
		CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval, MinSpawnInterval);
		UpdateSpawnTimer();
	}

	// Update ISM Instances
	if (ISMComponent && ISMComponent->GetStaticMesh())
	{
		bool bAnyUpdated = false;
		for (AEnemyBase* Enemy : ActiveEnemies)
		{
			if (IsValid(Enemy) && Enemy->GetISMInstanceIndex() != INDEX_NONE)
			{
				FTransform InstanceTransform;
				if (Enemy->IsHidden())
				{
					InstanceTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector);
				}
				else
				{
					InstanceTransform = Enemy->GetActorTransform();
					InstanceTransform.SetScale3D(InstanceTransform.GetScale3D() * ISMScaleMultiplier);
				}
				ISMComponent->UpdateInstanceTransform(Enemy->GetISMInstanceIndex(), InstanceTransform, true, false);
				bAnyUpdated = true;
			}
		}
		
		if (bAnyUpdated)
		{
			ISMComponent->MarkRenderStateDirty();
		}
	}

	// Cleanup distant enemies
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
	if (ISMComponent && Mesh)
	{
		ISMComponent->SetStaticMesh(Mesh);
		
		// Copy materials from mesh
		const TArray<FStaticMaterial>& StaticMaterials = Mesh->GetStaticMaterials();
		for (int32 i = 0; i < StaticMaterials.Num(); i++)
		{
			UMaterialInterface* Material = StaticMaterials[i].MaterialInterface;
			if (Material)
			{
				ISMComponent->SetMaterial(i, Material);
			}
		}
		
		ISMComponent->MarkRenderStateDirty();
		UE_LOG(LogTemp, Log, TEXT("ISM mesh set: %s"), *Mesh->GetName());
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

		int32 ISMIndex = Enemy->GetISMInstanceIndex();
		if (ISMIndex != INDEX_NONE)
		{
			ReleaseISMIndex(ISMIndex);
			Enemy->SetISMInstanceIndex(INDEX_NONE);
		}
	}
}

void UEnemySpawnSubsystem::SpawnEnemy()
{
	if (!EnemyClass || !ISMComponent)
	{
		return;
	}

	// Auto-setup ISM mesh from enemy class CDO (only once)
	if (!ISMComponent->GetStaticMesh())
	{
		if (const AEnemyBase* EnemyCDO = EnemyClass->GetDefaultObject<AEnemyBase>())
		{
			UStaticMesh* Mesh = EnemyCDO->GetEnemyMesh();
			if (Mesh)
			{
				SetEnemyMesh(Mesh);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("EnemyClass has no EnemyMesh set! Set EnemyMesh in the Blueprint."));
			}
		}
	}

	// Spawn batch
	int32 BaseBatchSize = 5;
	int32 TimeBonus = FMath::FloorToInt(GameTime / 30.0f);
	int32 EnemiesToSpawn = FMath::Min(BaseBatchSize + TimeBonus, 20);

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

			// ISM Integration
			if (ISMComponent && ISMComponent->GetStaticMesh())
			{
				if (Enemy->GetISMInstanceIndex() != INDEX_NONE)
				{
					ReleaseISMIndex(Enemy->GetISMInstanceIndex());
				}
				
				int32 NewIndex = AcquireISMIndex(Enemy->GetActorTransform());
				Enemy->SetISMInstanceIndex(NewIndex);
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
	float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	float RadAngle = FMath::DegreesToRadians(RandomAngle);
	float RandomDistance = FMath::RandRange(SpawnDistance * 0.8f, SpawnDistance * 1.2f);

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
		float HealthMultiplier = 1.0f + (GameTimeSeconds / 60.0f) * 0.1f;
		float BaseHealth = 100.0f;
		float ScaledHealth = BaseHealth * HealthMultiplier;

		Attributes->SetMaxHealth(ScaledHealth);
		Attributes->SetHealth(ScaledHealth);
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

	for (int32 i = ActiveEnemies.Num() - 1; i >= 0; i--)
	{
		AEnemyBase* Enemy = ActiveEnemies[i];
		
		if (!IsValid(Enemy))
		{
			ActiveEnemies.RemoveAt(i);
			continue;
		}

		float Distance = FVector::Dist(Enemy->GetActorLocation(), PlayerLocation);
		if (Distance > DespawnDistance)
		{
			Enemy->Deactivate();
		}
	}
}

void UEnemySpawnSubsystem::UpdateSpawnTimer()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&UEnemySpawnSubsystem::SpawnEnemy,
		CurrentSpawnInterval,
		true
	);
}

int32 UEnemySpawnSubsystem::AcquireISMIndex(const FTransform& Transform)
{
	if (!ISMComponent)
	{
		return INDEX_NONE;
	}

	FTransform ScaledTransform = Transform;
	ScaledTransform.SetScale3D(ScaledTransform.GetScale3D() * ISMScaleMultiplier);

	int32 ResultIndex = INDEX_NONE;

	// Try to reuse from free list
	if (FreeISMIndices.Num() > 0)
	{
		int32 ReuseIndex = FreeISMIndices.Pop(false);
		
		if (ReuseIndex >= 0 && ReuseIndex < ISMComponent->GetInstanceCount())
		{
			ISMComponent->UpdateInstanceTransform(ReuseIndex, ScaledTransform, true, true);
			ResultIndex = ReuseIndex;
		}
	}

	// Create new instance if needed
	if (ResultIndex == INDEX_NONE)
	{
		ResultIndex = ISMComponent->AddInstance(ScaledTransform, true);
	}

	if (ResultIndex != INDEX_NONE)
	{
		ISMComponent->MarkRenderStateDirty();
	}

	return ResultIndex;
}

void UEnemySpawnSubsystem::ReleaseISMIndex(int32 Index)
{
	if (!ISMComponent || Index == INDEX_NONE)
	{
		return;
	}

	if (Index < 0 || Index >= ISMComponent->GetInstanceCount())
	{
		return;
	}

	// Hide by scaling to zero
	ISMComponent->UpdateInstanceTransform(Index, FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector), true, true);
	FreeISMIndices.Add(Index);
}
