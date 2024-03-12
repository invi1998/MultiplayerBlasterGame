// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"

// Sets default values
APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;	// ��������

}

// Called when the game starts or when spawned
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	StartSpawnPickupTimer(nullptr);	// ��ʼ����ʰȡ�Ķ�ʱ��
	
}

void APickupSpawnPoint::SpawnPickup()
{
	if (PickupClasses.Num() > 0)	// ������ǵ�ʰȡ����������0�����ǾͿ�������һ��ʰȡ
	{
		UWorld* World = GetWorld();
		if (World)
		{
			int32 RandomIndex = FMath::RandRange(0, PickupClasses.Num() - 1);	// ����һ�������������ѡ��һ��ʰȡ��
			SpawnedPickup = World->SpawnActor<APickup>(PickupClasses[RandomIndex], GetActorTransform());	// ����һ��ʰȡ

			if (HasAuthority() && SpawnedPickup)
			{
				SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);	// ��ʰȡ������ʱ�����¿�ʼ����ʰȡ�Ķ�ʱ��
			}

		}
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())	// ����Ƿ�����
	{
		SpawnPickup();	// ����ʰȡ
		// StartSpawnPickupTimer();	// ���¿�ʼ����ʰȡ�Ķ�ʱ��
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestoryedActor)
{
	// ����һ�����ʱ�䣬��������ʰȡ
	const float SpawnTime = FMath::RandRange(SpawnTimeMin, SpawnTimeMax);
	// ���ö�ʱ������������ʰȡ
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnTime);
}

// Called every frame
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

