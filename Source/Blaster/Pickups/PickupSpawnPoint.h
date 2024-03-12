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
	TArray<TSubclassOf<class APickup>> PickupClasses;		// ���ڴ洢���е�Pickup��

	UPROPERTY(VisibleAnywhere)
	APickup* SpawnedPickup;		// ���ڴ洢���ɵ�Pickup

	void SpawnPickup();		// ����Pickup
	void SpawnPickupTimerFinished();		// ��ʱ������ʱ���õĺ���

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestoryedActor);			// ��ʼ����Pickup�Ķ�ʱ��

private:
	FTimerHandle SpawnPickupTimer;			// ��������Pickup�Ķ�ʱ��

	UPROPERTY(EditAnywhere)
	float SpawnTimeMin = 5.0f;				// ����Pickup����Сʱ��

	UPROPERTY(EditAnywhere)
	float SpawnTimeMax = 10.0f;				// ����Pickup�����ʱ��

};
