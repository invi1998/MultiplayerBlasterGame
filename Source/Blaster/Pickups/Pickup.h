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
		UPrimitiveComponent* OverlappedComponent,	// 该组件将被触发
		AActor* OtherActor,							// 与该组件发生重叠的Actor
		UPrimitiveComponent* OtherComp,			// 与该组件发生重叠的组件
		int32 OtherBodyIndex,						// 与该组件发生重叠的组件的BodyIndex
		bool bFromSweep,							// 是否是由Sweep引起的重叠
		const FHitResult& SweepResult				// Sweep引起的重叠的结果
	);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;		// 基础旋转速率 (deg/sec)

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;


	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectNiagaraComponent;		// 拾取特效

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffectNiagaraSystem;		// 拾取特效的NiagaraSystem

	FTimerHandle BindOverlapTimer;		// 绑定重叠事件的定时器

	float BindOverlapTime = 0.25f;		// 绑定重叠时间

	void BindOverlapTimerFinished();	// 绑定重叠事件的定时器结束时调用的函数

public:


};
