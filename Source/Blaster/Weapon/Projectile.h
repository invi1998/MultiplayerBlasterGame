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

	virtual void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 子弹命中逻辑处理函数
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

private:
	// 添加一个碰撞箱
	// 设置可编辑
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	// 示踪器 （显示子弹运动轨迹的粒子特效）
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;

	// 存储tracer的粒子系统组件
	class UParticleSystemComponent* TracerComponent;

	// 子弹命中的粒子特效
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;

	// 子弹命中的提示音
	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

public:
	// Called every frame
};
