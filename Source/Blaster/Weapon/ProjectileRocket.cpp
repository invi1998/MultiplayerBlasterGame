// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,	// 世界上下文对象
				Damage,	// 基础伤害
				10.f,	// 最小伤害
				GetActorLocation(),	// 伤害半径的圆心
				200.f,	// 伤害范围的内半径
				500.f,	// 伤害范围外半径
				1.0f,	// 伤害衰减方程（1表示线性衰减）
				UDamageType::StaticClass(),	// 伤害类型
				TArray<AActor*>(),	// 伤害忽略数组（设置为空数组，表示所有人都将会受到伤害，包括自己）
				this,	// 伤害发起者（自己）
				FiringController	// 伤害发起者的控制器
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
