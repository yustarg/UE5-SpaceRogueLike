// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceRoguelikePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SpaceShip.h"
#include "Blueprint/UserWidget.h"

ASpaceRoguelikePlayerController::ASpaceRoguelikePlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void ASpaceRoguelikePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Create and show HUD
	ShowHUD();
}

void ASpaceRoguelikePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Get Enhanced Input Component
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		// Bind Move Action
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceRoguelikePlayerController::OnMove);
		}

		// Bind Fire Action
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ASpaceRoguelikePlayerController::OnFirePressed);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ASpaceRoguelikePlayerController::OnFireReleased);
		}

		// Bind Pause Action
		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ASpaceRoguelikePlayerController::OnPause);
		}
	}
}

void ASpaceRoguelikePlayerController::OnMove(const FInputActionValue& Value)
{
	// Get 2D movement vector
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Forward/backward and Right/left movement
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		// Move in world space
		ControlledPawn->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), MovementVector.Y);
		ControlledPawn->AddMovementInput(FVector(0.0f, 1.0f, 0.0f), MovementVector.X);
	}
}

void ASpaceRoguelikePlayerController::OnFirePressed(const FInputActionValue& Value)
{
	ASpaceShip* SpaceShip = Cast<ASpaceShip>(GetPawn());
	if (SpaceShip)
	{
		SpaceShip->StartFiring();
	}
}

void ASpaceRoguelikePlayerController::OnFireReleased(const FInputActionValue& Value)
{
	ASpaceShip* SpaceShip = Cast<ASpaceShip>(GetPawn());
	if (SpaceShip)
	{
		SpaceShip->StopFiring();
	}
}

void ASpaceRoguelikePlayerController::OnPause(const FInputActionValue& Value)
{
	// TODO: Implement pause menu
	UE_LOG(LogTemp, Warning, TEXT("Pause - Not implemented yet"));
}

void ASpaceRoguelikePlayerController::ShowGameOverUI()
{
	// Hide HUD
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	// Show Game Over screen
	if (GameOverWidgetClass && !GameOverWidget)
	{
		GameOverWidget = CreateWidget<UUserWidget>(this, GameOverWidgetClass);
		if (GameOverWidget)
		{
			GameOverWidget->AddToViewport();
			
			// Show mouse cursor for restart button
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void ASpaceRoguelikePlayerController::ShowHUD()
{
	if (HUDWidgetClass && !HUDWidget)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}
