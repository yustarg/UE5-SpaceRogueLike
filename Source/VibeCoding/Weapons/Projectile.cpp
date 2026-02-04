// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projectile.h"
#include "EnemyBase.h"
#include "SpaceShipAttributeSet.h"
#include "ProjectilePoolSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create Collision Component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlapBegin);
	RootComponent = CollisionComponent;

	// Create Mesh Component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create Movement Component
	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = CollisionComponent;
	MovementComponent->InitialSpeed = 2000.0f;
	MovementComponent->MaxSpeed = 2000.0f;
	MovementComponent->ProjectileGravityScale = 0.0f; // No gravity in space
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;

	// Default values
	Speed = 2000.0f;
	Damage = 10.0f;
	Lifetime = 3.0f; // 3 seconds before auto-deactivate
}

void AProjectile::Activate(const FVector& Location, const FRotator& Rotation, AActor* InOwner)
{
	SetActorLocationAndRotation(Location, Rotation);
	SetOwner(InOwner);
	
	// Ignore owner collision
	if (InOwner && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(InOwner, true);
	}

	// Reset and show
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	
	if (MovementComponent)
	{
		MovementComponent->SetUpdatedComponent(CollisionComponent);
		MovementComponent->Velocity = Rotation.Vector() * Speed;
		MovementComponent->UpdateComponentVelocity();
		MovementComponent->Activate(true);
	}

	// Set deactivation timer (replaces LifeSpan)
	GetWorldTimerManager().SetTimer(DeactivateTimerHandle, this, &AProjectile::Deactivate, Lifetime);
}

void AProjectile::Deactivate()
{
	// Clear ignore list
	if (AActor* MyOwner = GetOwner())
	{
		if (CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(MyOwner, false);
		}
	}

	// Hide and disable
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->Deactivate();
	}

	GetWorldTimerManager().ClearTimer(DeactivateTimerHandle);

	// Return to pool
	if (UWorld* World = GetWorld())
	{
		if (UProjectilePoolSubsystem* PoolSubsystem = World->GetSubsystem<UProjectilePoolSubsystem>())
		{
			PoolSubsystem->ReturnToPool(this);
		}
	}
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Initially deactivated if spawned via pool, but if spawned normally we might want it active
	// For MVP, we'll let Activate handle it
}

void AProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Don't hit ourselves or our owner
	if (OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// Check if hit an enemy
	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	if (Enemy)
	{
		UAbilitySystemComponent* TargetASC = Enemy->GetAbilitySystemComponent();
		if (TargetASC)
		{
			// Apply damage directly to AttributeSet (MVP simplified)
			USpaceShipAttributeSet* TargetAttributes = const_cast<USpaceShipAttributeSet*>(
				TargetASC->GetSet<USpaceShipAttributeSet>()
			);
			
			if (TargetAttributes)
			{
				float CurrentHealth = TargetAttributes->GetHealth();
				TargetAttributes->SetHealth(CurrentHealth - Damage);
			}
		}

		// Deactivate after hitting enemy
		Deactivate();
	}
}
