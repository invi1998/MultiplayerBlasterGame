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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// �ӵ������߼�������
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	// ���һ����ײ��
	// ���ÿɱ༭
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	// ʾ���� ����ʾ�ӵ��˶��켣��������Ч��
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;

	// �洢tracer������ϵͳ���
	class UParticleSystemComponent* TracerComponent;

	// �ӵ����е�������Ч
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;

	// �ӵ����е���ʾ��
	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

public:
	// Called every frame
};
