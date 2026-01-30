// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyBase.h"
#include "AbilitySystemComponent.h"
#include "SpaceShipAttributeSet.h"
#include "SpaceRoguelikeGameState.h"
#include "SpaceShip.h"
#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Hide default skeletal mesh (we don't need skeletal animation for 2D top-down game)
	// Add a Static Mesh component in Blueprint instead for better performance
	GetMesh()->SetVisibility(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<USpaceShipAttributeSet>(TEXT("AttributeSet"));

	// Configure Character Movement
	GetCharacterMovement()->GravityScale = 0.0f; // No gravity in space
	GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Same speed as player
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f));

	// Configure AI
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Default values
	ContactDamage = 10.0f;
	bIsDead = false;
	bHasDealtDamage = false;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Ability System
	if (AbilitySystemComponent)
	{
		// Listen to Health changes
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()
		).AddUObject(this, &AEnemyBase::OnHealthChanged);
	}

	// Setup overlap for contact damage
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnOverlapBegin);
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* AEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemyBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Log, TEXT("Enemy Health: %.2f / %.2f"), Data.NewValue, AttributeSet->GetMaxHealth());

	// Check for death
	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		Die();
	}
}

void AEnemyBase::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	UE_LOG(LogTemp, Log, TEXT("Enemy Died!"));

	// Increment kill count in GameState
	ASpaceRoguelikeGameState* GameState = GetWorld()->GetGameState<ASpaceRoguelikeGameState>();
	if (GameState)
	{
		GameState->AddKill();
	}

	// TODO: Play death effects
	// TODO: Spawn loot

	// Destroy actor
	Destroy();
}

void AEnemyBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsDead || bHasDealtDamage)
	{
		return;
	}

	// Check if overlapped with player
	ASpaceShip* Player = Cast<ASpaceShip>(OtherActor);
	if (Player)
	{
		UAbilitySystemComponent* TargetASC = Player->GetAbilitySystemComponent();
		if (TargetASC)
		{
			// Create damage effect context
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			// Create a simple instant damage effect
			// Note: In production, this should use a proper GameplayEffect class
			// For MVP, we'll directly modify health
			USpaceShipAttributeSet* TargetAttributes = const_cast<USpaceShipAttributeSet*>(
				TargetASC->GetSet<USpaceShipAttributeSet>()
			);
			
			if (TargetAttributes)
			{
				float CurrentHealth = TargetAttributes->GetHealth();
				TargetAttributes->SetHealth(CurrentHealth - ContactDamage);
				
				UE_LOG(LogTemp, Warning, TEXT("Enemy dealt %f damage to player!"), ContactDamage);
			}

			bHasDealtDamage = true;
		}

		// Destroy self after dealing damage
		Die();
	}
}
