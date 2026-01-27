// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projectile.h"
#include "EnemyBase.h"
#include "SpaceShipAttributeSet.h"
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
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
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
	Lifetime = 3.0f; // 3 seconds before auto-destroy

	// Set lifespan
	InitialLifeSpan = Lifetime;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Apply speed
	if (MovementComponent)
	{
		MovementComponent->InitialSpeed = Speed;
		MovementComponent->MaxSpeed = Speed;
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
			// In production, use GameplayEffect
			USpaceShipAttributeSet* TargetAttributes = const_cast<USpaceShipAttributeSet*>(
				TargetASC->GetSet<USpaceShipAttributeSet>()
			);
			
			if (TargetAttributes)
			{
				float CurrentHealth = TargetAttributes->GetHealth();
				TargetAttributes->SetHealth(CurrentHealth - Damage);
				
				UE_LOG(LogTemp, Log, TEXT("Projectile dealt %f damage!"), Damage);
			}
		}

		// TODO: Play hit effects
	}

	// Destroy projectile
	Destroy();
}
