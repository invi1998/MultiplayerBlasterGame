// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// ȷ���ӵ���������ת���ٶȶ���
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

	FPredictProjectilePathParams PredictedPath;		// Ԥ�ⵯ��·������
	PredictedPath.bTraceWithCollision = true;	// �Ƿ�ʹ����ײ���
	PredictedPath.bTraceWithChannel = true;	// �Ƿ�ʹ��ͨ�����
	PredictedPath.DrawDebugType = EDrawDebugTrace::ForDuration;	// ���Ƶ������ͣ�����ʱ��
	PredictedPath.DrawDebugTime = 5.f;	// ���Ƶ���ʱ��
	PredictedPath.LaunchVelocity = GetActorForwardVector() * InitialSpeed;	// �����ٶ�
	PredictedPath.MaxSimTime = 5.f;	// ���ģ��ʱ��
	PredictedPath.ProjectileRadius = 5.f;	// ����뾶
	PredictedPath.StartLocation = GetActorLocation();	// ��ʼλ��
	PredictedPath.SimFrequency = 30.f;	// ģ��Ƶ��
	PredictedPath.StartLocation = GetActorLocation();	// ��ʼλ��
	PredictedPath.TraceChannel = ECC_Visibility;	// ����ͨ��
	PredictedPath.ActorsToIgnore.Add(this);	// ���ԵĽ�ɫ

	FPredictProjectilePathResult PredictedPathResult;	// Ԥ�ⵯ��·�����


	UGameplayStatics::PredictProjectilePath(this, PredictedPath, PredictedPathResult);
}
