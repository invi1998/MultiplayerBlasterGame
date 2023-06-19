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
				this,	// ���������Ķ���
				Damage,	// �����˺�
				10.f,	// ��С�˺�
				GetActorLocation(),	// �˺��뾶��Բ��
				200.f,	// �˺���Χ���ڰ뾶
				500.f,	// �˺���Χ��뾶
				1.0f,	// �˺�˥�����̣�1��ʾ����˥����
				UDamageType::StaticClass(),	// �˺�����
				TArray<AActor*>(),	// �˺��������飨����Ϊ�����飬��ʾ�����˶������ܵ��˺��������Լ���
				this,	// �˺������ߣ��Լ���
				FiringController	// �˺������ߵĿ�����
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
