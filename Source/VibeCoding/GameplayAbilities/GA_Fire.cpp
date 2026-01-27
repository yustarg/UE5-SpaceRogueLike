// Copyright Epic Games, Inc. All Rights Reserved.

#include "GA_Fire.h"
#include "Projectile.h"
#include "SpaceShipAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

UGA_Fire::UGA_Fire()
{
	// Set defaults
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	SpawnOffset = 100.0f;
}

void UGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get owner actor
	AActor* Owner = ActorInfo->AvatarActor.Get();
	if (!Owner)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Calculate spawn location (in front of ship)
	FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * SpawnOffset;
	FRotator SpawnRotation = Owner->GetActorRotation();

	// Spawn projectile
	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Cast<APawn>(Owner);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (Projectile)
		{
			// Get damage from AttributeSet
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
			if (ASC)
			{
				const USpaceShipAttributeSet* Attributes = ASC->GetSet<USpaceShipAttributeSet>();
				if (Attributes)
				{
					float DamageValue = Attributes->GetDamage();
					Projectile->SetDamage(DamageValue);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Projectile spawned at %s"), *SpawnLocation.ToString());
		}
	}

	// Play fire sound
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, SpawnLocation);
	}

	// End ability
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
