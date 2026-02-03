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

	/** Activate the enemy from pool */
	virtual void Activate(const FVector& Location);

	/** Deactivate and return to pool */
	virtual void Deactivate();

	/** HISM instance index accessors */
	int32 GetHISMInstanceIndex() const { return HISMInstanceIndex; }
	void SetHISMInstanceIndex(int32 Index) { HISMInstanceIndex = Index; }

	/** Get the static mesh used for HISM rendering */
	UFUNCTION(BlueprintPure, Category = "Rendering")
	UStaticMesh* GetEnemyMesh() const { return EnemyMesh; }

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

	/** Index of this enemy in the HISM component */
	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	int32 HISMInstanceIndex;

	/** Static mesh used for HISM rendering (set this in Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rendering")
	UStaticMesh* EnemyMesh;

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
