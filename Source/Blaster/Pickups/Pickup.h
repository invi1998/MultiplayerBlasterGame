// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();
	virtual void Destroyed() override;

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,	// �������������
		AActor* OtherActor,							// �����������ص���Actor
		UPrimitiveComponent* OtherComp,			// �����������ص������
		int32 OtherBodyIndex,						// �����������ص��������BodyIndex
		bool bFromSweep,							// �Ƿ�����Sweep������ص�
		const FHitResult& SweepResult				// Sweep������ص��Ľ��
	);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;		// ������ת���� (deg/sec)

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;


	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectNiagaraComponent;		// ʰȡ��Ч

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffectNiagaraSystem;		// ʰȡ��Ч��NiagaraSystem

	FTimerHandle BindOverlapTimer;		// ���ص��¼��Ķ�ʱ��

	float BindOverlapTime = 0.25f;		// ���ص�ʱ��

	void BindOverlapTimerFinished();	// ���ص��¼��Ķ�ʱ������ʱ���õĺ���

public:


};
