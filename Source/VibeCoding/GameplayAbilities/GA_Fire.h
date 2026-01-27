// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Fire.generated.h"

class AProjectile;

/**
 * Fire Ability - Spawns a projectile
 * Used by SpaceShip to shoot
 */
UCLASS()
class VIBECODING_API UGA_Fire : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Fire();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	/** Projectile spawn offset (forward from ship) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float SpawnOffset;

	/** Fire sound */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* FireSound;
};
