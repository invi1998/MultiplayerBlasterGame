// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// 确保子弹保持其旋转与速度对齐
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (AController* OwnerController = OwnerCharacter->Controller)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PredictedPath;		// 预测弹道路径参数
	PredictedPath.bTraceWithCollision = true;	// 是否使用碰撞检测
	PredictedPath.bTraceWithChannel = true;	// 是否使用通道检测
	PredictedPath.DrawDebugType = EDrawDebugTrace::ForDuration;	// 绘制调试类型，持续时间
	PredictedPath.DrawDebugTime = 5.f;	// 绘制调试时间
	PredictedPath.LaunchVelocity = GetActorForwardVector() * InitialSpeed;	// 发射速度
	PredictedPath.MaxSimTime = 5.f;	// 最大模拟时间
	PredictedPath.ProjectileRadius = 5.f;	// 弹丸半径
	PredictedPath.StartLocation = GetActorLocation();	// 起始位置
	PredictedPath.SimFrequency = 30.f;	// 模拟频率
	PredictedPath.StartLocation = GetActorLocation();	// 起始位置
	PredictedPath.TraceChannel = ECC_Visibility;	// 跟踪通道
	PredictedPath.ActorsToIgnore.Add(this);	// 忽略的角色

	FPredictProjectilePathResult PredictedPathResult;	// 预测弹道路径结果


	UGameplayStatics::PredictProjectilePath(this, PredictedPath, PredictedPathResult);
}
