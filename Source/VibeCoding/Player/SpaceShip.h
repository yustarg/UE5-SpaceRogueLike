// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "SpaceShip.generated.h"

struct FOnAttributeChangeData;

class UAbilitySystemComponent;
class USpaceShipAttributeSet;
class UGameplayAbility;
class UCameraComponent;
class USpringArmComponent;

/**
 * Player-controlled SpaceShip Character
 * Uses GAS for attributes and abilities
 */
UCLASS()
class VIBECODING_API ASpaceShip : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASpaceShip();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//=============================================================================
	// Firing
	//=============================================================================
	
	/** Start continuous firing */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartFiring();

	/** Stop firing */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopFiring();

protected:
	//=============================================================================
	// Components
	//=============================================================================
	
	/** Ability System Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	/** Attribute Set */
	UPROPERTY()
	USpaceShipAttributeSet* AttributeSet;

	/** Camera Boom */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/** Top-down Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* TopDownCamera;

	//=============================================================================
	// Abilities
	//=============================================================================
	
	/** Fire Ability Class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> FireAbilityClass;

	/** Handle to fire ability */
	FGameplayAbilitySpecHandle FireAbilityHandle;

	//=============================================================================
	// Combat
	//=============================================================================
	
	/** Timer for continuous firing */
	FTimerHandle FireTimerHandle;

	/** Fire rate (seconds between shots) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float FireRate;

	/** Fire a single shot (calls GAS Ability) */
	void Fire();

	//=============================================================================
	// Health & Death
	//=============================================================================
	
	/** Called when health changes */
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	/** Called when character dies */
	virtual void Die();

	/** Is the character dead? */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead;

	//=============================================================================
	// Mouse Rotation
	//=============================================================================
	
	/** Rotate ship to face mouse cursor */
	void RotateTowardsCursor(float DeltaTime);

	/** Rotation speed */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RotationSpeed;
};
