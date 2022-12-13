// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

// 弹丸/子弹类

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	// 示踪器 （显示子弹运动轨迹的粒子特效）
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;

	// 存储tracer的粒子系统组件
	class UParticleSystemComponent* TracerComponent;

private:
	// 添加一个碰撞箱
	// 设置可编辑
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

public:
	// Called every frame
};
