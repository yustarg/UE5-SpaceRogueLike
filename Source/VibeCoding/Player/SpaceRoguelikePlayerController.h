// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "SpaceRoguelikePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;

/**
 * Player Controller for Space Roguelike
 * Handles input and UI management
 */
UCLASS()
class VIBECODING_API ASpaceRoguelikePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASpaceRoguelikePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	/** Show game over UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOverUI();

	/** Show HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUD();

protected:
	//=============================================================================
	// Input Mapping
	//=============================================================================
	
	/** Input Mapping Context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** Fire Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	/** Pause Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* PauseAction;

	//=============================================================================
	// Input Callbacks
	//=============================================================================
	
	void OnMove(const FInputActionValue& Value);
	void OnFirePressed(const FInputActionValue& Value);
	void OnFireReleased(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);

	//=============================================================================
	// UI
	//=============================================================================
	
	/** HUD Widget Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/** Game Over Widget Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	/** Current HUD Widget */
	UPROPERTY()
	UUserWidget* HUDWidget;

	/** Current Game Over Widget */
	UPROPERTY()
	UUserWidget* GameOverWidget;
};
