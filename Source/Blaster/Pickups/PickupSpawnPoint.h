// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;		// 用于存储所有的Pickup类

	UPROPERTY(VisibleAnywhere)
	APickup* SpawnedPickup;		// 用于存储生成的Pickup

	void SpawnPickup();		// 生成Pickup
	void SpawnPickupTimerFinished();		// 定时器结束时调用的函数

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestoryedActor);			// 开始生成Pickup的定时器

private:
	FTimerHandle SpawnPickupTimer;			// 用于生成Pickup的定时器

	UPROPERTY(EditAnywhere)
	float SpawnTimeMin = 5.0f;				// 生成Pickup的最小时间

	UPROPERTY(EditAnywhere)
	float SpawnTimeMax = 10.0f;				// 生成Pickup的最大时间

};
