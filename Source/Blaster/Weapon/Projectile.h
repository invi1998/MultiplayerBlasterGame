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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	// ʾ���� ����ʾ�ӵ��˶��켣��������Ч��
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;

	// �洢tracer������ϵͳ���
	class UParticleSystemComponent* TracerComponent;

private:
	// ���һ����ײ��
	// ���ÿɱ༭
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

public:
	// Called every frame
};
