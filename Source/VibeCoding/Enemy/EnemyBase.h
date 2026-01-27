// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "EnemyBase.generated.h"

struct FOnAttributeChangeData;

class UAbilitySystemComponent;
class USpaceShipAttributeSet;

/**
 * Base Enemy Character
 * Uses GAS for attributes, simple AI for chasing player
 */
UCLASS()
class VIBECODING_API AEnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Get Attribute Set */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	USpaceShipAttributeSet* GetAttributeSet() const { return AttributeSet; }

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

	//=============================================================================
	// Health & Death
	//=============================================================================
	
	/** Called when health changes */
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	/** Called when enemy dies */
	virtual void Die();

	/** Is the enemy dead? */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead;

	//=============================================================================
	// Combat (Contact Damage)
	//=============================================================================
	
	/** Damage dealt on contact */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float ContactDamage;

	/** Has already dealt damage (to prevent multiple hits) */
	bool bHasDealtDamage;

	/** Handle overlap with player */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
		const FHitResult& SweepResult);
};
