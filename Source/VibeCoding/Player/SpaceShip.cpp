// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShip.h"
#include "AbilitySystemComponent.h"
#include "SpaceShipAttributeSet.h"
#include "SpaceRoguelikeGameMode.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASpaceShip::ASpaceShip()
{
	PrimaryActorTick.bCanEverTick = true;

	// Hide default skeletal mesh (we don't need skeletal animation for 2D top-down game)
	// Add a Static Mesh component in Blueprint instead for better performance
	GetMesh()->SetVisibility(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<USpaceShipAttributeSet>(TEXT("AttributeSet"));

	// Create Camera Boom (for top-down view)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1500.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false; // Don't test collision in space
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;

	// Create Camera
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;

	// Configure Character Movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // We'll handle rotation manually
	GetCharacterMovement()->GravityScale = 0.0f; // No gravity in space
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	
	// Constrain movement to XY plane
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f));

	// Default values
	FireRate = 0.2f; // 5 shots per second
	RotationSpeed = 10.0f;
	bIsDead = false;
}

void ASpaceShip::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Ability System
	if (AbilitySystemComponent)
	{
		// Grant Fire Ability
		if (FireAbilityClass)
		{
			FireAbilityHandle = AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(FireAbilityClass, 1, INDEX_NONE, this)
			);
		}

		// Listen to Health changes
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()
		).AddUObject(this, &ASpaceShip::OnHealthChanged);
	}
}

void ASpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDead)
	{
		// Rotate towards mouse cursor
		RotateTowardsCursor(DeltaTime);
	}
}

void ASpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Input is handled by PlayerController
}

UAbilitySystemComponent* ASpaceShip::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASpaceShip::StartFiring()
{
	if (bIsDead)
	{
		return;
	}

	// Fire immediately
	Fire();

	// Start timer for continuous firing
	GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ASpaceShip::Fire,
		FireRate,
		true
	);
}

void ASpaceShip::StopFiring()
{
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void ASpaceShip::Fire()
{
	if (AbilitySystemComponent && FireAbilityHandle.IsValid())
	{
		// Try to activate fire ability
		AbilitySystemComponent->TryActivateAbility(FireAbilityHandle);
	}
}

void ASpaceShip::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Log, TEXT("SpaceShip Health: %.2f / %.2f"), Data.NewValue, AttributeSet->GetMaxHealth());

	// Check for death
	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		Die();
	}
}

void ASpaceShip::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	StopFiring();

	UE_LOG(LogTemp, Warning, TEXT("SpaceShip Died!"));

	// Disable movement
	GetCharacterMovement()->DisableMovement();

	// Notify Game Mode
	ASpaceRoguelikeGameMode* GameMode = Cast<ASpaceRoguelikeGameMode>(
		UGameplayStatics::GetGameMode(this)
	);
	if (GameMode)
	{
		GameMode->OnPlayerDeath(GetController());
	}

	// TODO: Play death effects
}

void ASpaceShip::RotateTowardsCursor(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// Get mouse position in world
	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (HitResult.bBlockingHit)
	{
		// Calculate direction to cursor
		FVector CursorLocation = HitResult.Location;
		FVector ShipLocation = GetActorLocation();
		FVector Direction = (CursorLocation - ShipLocation).GetSafeNormal2D();

		// Calculate target rotation
		FRotator TargetRotation = Direction.Rotation();

		// Smoothly interpolate to target rotation
		FRotator NewRotation = FMath::RInterpTo(
			GetActorRotation(),
			TargetRotation,
			DeltaTime,
			RotationSpeed
		);

		SetActorRotation(NewRotation);
	}
}
