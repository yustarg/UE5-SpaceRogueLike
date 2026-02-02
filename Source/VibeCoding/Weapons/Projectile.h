// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

/**
 * Projectile Actor - Bullet fired by player
 * Uses UE5's ProjectileMovementComponent
 */
UCLASS()
class VIBECODING_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

	/** Activate the projectile from pool */
	virtual void Activate(const FVector& Location, const FRotator& Rotation, AActor* InOwner);

	/** Deactivate and return to pool */
	virtual void Deactivate();

protected:
	virtual void BeginPlay() override;

	/** Timer for auto-deactivation */
	FTimerHandle DeactivateTimerHandle;

public:
	/** Set damage value */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetDamage(float InDamage) { Damage = InDamage; }

	/** Get damage value */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetDamage() const { return Damage; }

protected:
	//=============================================================================
	// Components
	//=============================================================================
	
	/** Collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	/** Mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* MovementComponent;

	//=============================================================================
	// Combat
	//=============================================================================
	
	/** Damage dealt to targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	/** Projectile speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float Speed;

	/** Projectile lifetime (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Lifetime;

	//=============================================================================
	// Collision
	//=============================================================================
	
	/** Handle collision with target */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
