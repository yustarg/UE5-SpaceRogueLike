// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "EnergyShield.generated.h"

UCLASS()
class VIBECODING_API AEnergyShield : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnergyShield();

protected:
	virtual void BeginPlay() override;

	// 能量罩网格组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ShieldMesh;

	// 碰撞检测组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	// 破碎粒子效果
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> BreakParticles;

	// 能量流动粒子
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> EnergyFlowParticles;

	// 材质参数集合（用于控制全局效果）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield Settings")
	TObjectPtr<UMaterialParameterCollection> ShieldMaterialParams;

	// 击碎动画时长
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float BreakDuration = 1.5f;

	// 能量罩强度（0-1）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShieldStrength = 1.0f;

	// 是否正在击碎
	UPROPERTY(BlueprintReadOnly, Category = "Shield State")
	bool bIsBreaking = false;

	// 击碎进度（0-1）
	UPROPERTY(BlueprintReadOnly, Category = "Shield State")
	float BreakProgress = 0.0f;

public:
	// 开始击碎效果
	UFUNCTION(BlueprintCallable, Category = "Shield")
	void StartBreak();

	// 设置能量罩强度
	UFUNCTION(BlueprintCallable, Category = "Shield")
	void SetShieldStrength(float NewStrength);

	// 重置能量罩
	UFUNCTION(BlueprintCallable, Category = "Shield")
	void ResetShield();

	// 碰撞事件
	UFUNCTION()
	void OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	// 击碎动画时间线
	FTimerHandle BreakTimerHandle;

	// 更新击碎动画
	void UpdateBreakAnimation();

	// 完成击碎
	void OnBreakComplete();

	// 初始缩放
	FVector InitialScale;
};
