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
	// ȷ���ӵ���������ת���ٶȶ���
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;	// ��ȡ��������

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))	// �ж����������Ƿ�Ϊ��ʼ�ٶ�
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;	// ���ó�ʼ�ٶ�
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;	// ��������ٶ�
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
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)	// ����ˣ���ʹ�÷���˵���
			{
				// ����ˣ���ʹ�÷���˵���
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)	// ��Ҫʹ�÷���˵���������ӵ�з���˵�������������Ǳ��ؿ��ƣ����н�ɫ��Ϊ��
			{
				// ʹ�÷���˵���
				OwnerCharacter->GetLagCompensation()->ServerScoreRequest_Projectile(
					HitCharacter, 
					TraceStart, 
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime, 
					nullptr);		// �˺���Դ������գ�Ҳ���Դ����ɫ��ǰװ������������������Ҫ����Ϊ����������Զ���ȡ�˺���Դ
			}
			
		}
	}

	
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * ���д�������Ԥ�ⵯ��·����debug����
	 */
	//FPredictProjectilePathParams PredictedPath;		// Ԥ�ⵯ��·������
	//PredictedPath.bTraceWithCollision = true;	// �Ƿ�ʹ����ײ���
	//PredictedPath.bTraceWithChannel = true;	// �Ƿ�ʹ��ͨ�����
	//PredictedPath.DrawDebugType = EDrawDebugTrace::ForDuration;	// ���Ƶ������ͣ�����ʱ��
	//PredictedPath.DrawDebugTime = 5.f;	// ���Ƶ���ʱ��
	//PredictedPath.LaunchVelocity = GetActorForwardVector() * InitialSpeed;	// �����ٶ�
	//PredictedPath.MaxSimTime = 5.f;	// ���ģ��ʱ��
	//PredictedPath.ProjectileRadius = 5.f;	// ����뾶
	//PredictedPath.StartLocation = GetActorLocation();	// ��ʼλ��
	//PredictedPath.SimFrequency = 30.f;	// ģ��Ƶ��
	//PredictedPath.StartLocation = GetActorLocation();	// ��ʼλ��
	//PredictedPath.TraceChannel = ECC_Visibility;	// ����ͨ��
	//PredictedPath.ActorsToIgnore.Add(this);	// ���ԵĽ�ɫ

	//FPredictProjectilePathResult PredictedPathResult;	// Ԥ�ⵯ��·�����


	//UGameplayStatics::PredictProjectilePath(this, PredictedPath, PredictedPathResult);
}
