// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

// ����/�ӵ���

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	/*
	 * ���ڷ���˵���
	 */
	bool bUseServerSideRewind = true;		// �Ƿ�ʹ�÷���˵���
	FVector_NetQuantize TraceStart;			// ����˵���λ��
	FVector_NetQuantize100 InitialVelocity;	// ����˵�����ʼ�ٶȣ�����Ϊ0.01

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;		// ��ʼ�ٶ�
	
	float Damage = 20.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	// �ӵ������߼�������
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// �ӵ����е�������Ч
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	// �ӵ����е���ʾ��
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	// ���һ����ײ��
	// ���ÿɱ༭
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;


	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	// ��ը����˺���Χ�ڰ뾶
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	// ��ը����˺���Χ��뾶
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:

	// ʾ���� ����ʾ�ӵ��˶��켣��������Ч��
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	// �洢tracer������ϵͳ���
	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;

public:
	// Called every frame
};
