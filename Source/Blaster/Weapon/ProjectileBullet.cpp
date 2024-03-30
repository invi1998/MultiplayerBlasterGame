// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBullet.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
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

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;	// 获取属性名称

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))	// 判断属性名称是否为初始速度
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;	// 设置初始速度
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;	// 设置最大速度
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (const ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()))
	{
		if (ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller))
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)	// 服务端，不使用服务端倒带
			{
				// 服务端，不使用服务端倒带
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)	// 需要使用服务端倒带，并且拥有服务端倒带组件，并且是本地控制，命中角色不为空
			{
				// 使用服务端倒带
				OwnerCharacter->GetLagCompensation()->ServerScoreRequest_Projectile(
					HitCharacter, 
					TraceStart, 
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime, 
					nullptr);		// 伤害来源这里填空，也可以传入角色当前装备的武器，不过不重要，因为倒带组件会自动获取伤害来源
			}
			
		}
	}

	
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * 下列代码用于预测弹道路径，debug绘制
	 */
	//FPredictProjectilePathParams PredictedPath;		// 预测弹道路径参数
	//PredictedPath.bTraceWithCollision = true;	// 是否使用碰撞检测
	//PredictedPath.bTraceWithChannel = true;	// 是否使用通道检测
	//PredictedPath.DrawDebugType = EDrawDebugTrace::ForDuration;	// 绘制调试类型，持续时间
	//PredictedPath.DrawDebugTime = 5.f;	// 绘制调试时间
	//PredictedPath.LaunchVelocity = GetActorForwardVector() * InitialSpeed;	// 发射速度
	//PredictedPath.MaxSimTime = 5.f;	// 最大模拟时间
	//PredictedPath.ProjectileRadius = 5.f;	// 弹丸半径
	//PredictedPath.StartLocation = GetActorLocation();	// 起始位置
	//PredictedPath.SimFrequency = 30.f;	// 模拟频率
	//PredictedPath.StartLocation = GetActorLocation();	// 起始位置
	//PredictedPath.TraceChannel = ECC_Visibility;	// 跟踪通道
	//PredictedPath.ActorsToIgnore.Add(this);	// 忽略的角色

	//FPredictProjectilePathResult PredictedPathResult;	// 预测弹道路径结果


	//UGameplayStatics::PredictProjectilePath(this, PredictedPath, PredictedPathResult);
}
