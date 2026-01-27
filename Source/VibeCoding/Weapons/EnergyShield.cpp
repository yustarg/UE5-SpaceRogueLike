// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnergyShield.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AEnergyShield::AEnergyShield()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 创建碰撞球体
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(100.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->OnComponentHit.AddDynamic(this, &AEnergyShield::OnShieldHit);

	// 创建能量罩网格
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetCastShadow(false);

	// 创建破碎粒子
	BreakParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BreakParticles"));
	BreakParticles->SetupAttachment(RootComponent);
	BreakParticles->bAutoActivate = false;

	// 创建能量流动粒子
	EnergyFlowParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EnergyFlowParticles"));
	EnergyFlowParticles->SetupAttachment(RootComponent);
	EnergyFlowParticles->bAutoActivate = true;

	// 默认值
	BreakDuration = 1.5f;
	ShieldStrength = 1.0f;
	bIsBreaking = false;
	BreakProgress = 0.0f;
	InitialScale = FVector(1.0f, 1.0f, 1.0f);
}

void AEnergyShield::BeginPlay()
{
	Super::BeginPlay();
	
	InitialScale = ShieldMesh->GetRelativeScale3D();
	
	// 激活能量流动粒子
	if (EnergyFlowParticles)
	{
		EnergyFlowParticles->Activate();
	}
}

void AEnergyShield::StartBreak()
{
	if (bIsBreaking)
	{
		return;
	}

	bIsBreaking = true;
	BreakProgress = 0.0f;

	// 激活破碎粒子
	if (BreakParticles)
	{
		BreakParticles->Activate();
	}

	// 停止能量流动粒子
	if (EnergyFlowParticles)
	{
		EnergyFlowParticles->Deactivate();
	}

	// 设置定时器更新动画
	GetWorldTimerManager().SetTimer(
		BreakTimerHandle,
		this,
		&AEnergyShield::UpdateBreakAnimation,
		0.016f, // 约60fps更新
		true
	);
}

void AEnergyShield::UpdateBreakAnimation()
{
	BreakProgress += 0.016f / BreakDuration;

	if (BreakProgress >= 1.0f)
	{
		BreakProgress = 1.0f;
		OnBreakComplete();
		return;
	}

	// 更新材质参数
	if (ShieldMaterialParams)
	{
		UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(ShieldMaterialParams);
		if (Instance)
		{
			// 设置破碎进度参数
			Instance->SetScalarParameterValue(FName("BreakProgress"), BreakProgress);
			Instance->SetScalarParameterValue(FName("ShieldStrength"), 1.0f - BreakProgress);
		}
	}

	// 更新网格缩放（破碎时缩小）
	float ScaleFactor = FMath::Lerp(1.0f, 0.0f, BreakProgress);
	ShieldMesh->SetRelativeScale3D(InitialScale * ScaleFactor);

	// 更新透明度
	if (ShieldMesh && ShieldMesh->GetMaterial(0))
	{
		// 通过动态材质实例更新
		UMaterialInstanceDynamic* DynamicMaterial = ShieldMesh->CreateDynamicMaterialInstance(0);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(FName("Opacity"), 1.0f - BreakProgress);
		}
	}
}

void AEnergyShield::OnBreakComplete()
{
	GetWorldTimerManager().ClearTimer(BreakTimerHandle);
	
	// 隐藏网格
	ShieldMesh->SetVisibility(false);
	
	// 禁用碰撞
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnergyShield::SetShieldStrength(float NewStrength)
{
	ShieldStrength = FMath::Clamp(NewStrength, 0.0f, 1.0f);

	if (ShieldMaterialParams)
	{
		UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(ShieldMaterialParams);
		if (Instance)
		{
			Instance->SetScalarParameterValue(FName("ShieldStrength"), ShieldStrength);
		}
	}
}

void AEnergyShield::ResetShield()
{
	bIsBreaking = false;
	BreakProgress = 0.0f;
	
	GetWorldTimerManager().ClearTimer(BreakTimerHandle);
	
	// 恢复网格
	ShieldMesh->SetVisibility(true);
	ShieldMesh->SetRelativeScale3D(InitialScale);
	
	// 恢复碰撞
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 重置材质参数
	if (ShieldMaterialParams)
	{
		UMaterialInstanceDynamic* DynamicMaterial = ShieldMesh->CreateDynamicMaterialInstance(0);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(FName("Opacity"), 1.0f);
		}
		
		UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(ShieldMaterialParams);
		if (Instance)
		{
			Instance->SetScalarParameterValue(FName("BreakProgress"), 0.0f);
			Instance->SetScalarParameterValue(FName("ShieldStrength"), 1.0f);
		}
	}
	
	// 重新激活能量流动粒子
	if (EnergyFlowParticles)
	{
		EnergyFlowParticles->Activate();
	}
}

void AEnergyShield::OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 当被击中时，可以触发击碎效果
	// 这里可以根据需要添加条件判断
	if (!bIsBreaking)
	{
		StartBreak();
	}
}
