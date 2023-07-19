// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);

}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.0f,
			1.0f,
			0.f,
			LoopingSoundAttenuation,
			static_cast<USoundConcurrency*>(nullptr),
			false
		);
	}

}

void AProjectileRocket::Destroyed()
{
	
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Hit self"));
		return;
	}

	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
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

	StartDestroyTimer();

	if (ImpactParticles)
	{
		// ������������
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		// ����������ʾ��
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	// Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

