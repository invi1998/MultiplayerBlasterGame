// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"

// Sets default values
APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;	// 开启复制

}

// Called when the game starts or when spawned
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	StartSpawnPickupTimer(nullptr);	// 开始生成拾取的定时器
	
}

void APickupSpawnPoint::SpawnPickup()
{
	if (PickupClasses.Num() > 0)	// 如果我们的拾取类数量大于0，我们就可以生成一个拾取
	{
		UWorld* World = GetWorld();
		if (World)
		{
			int32 RandomIndex = FMath::RandRange(0, PickupClasses.Num() - 1);	// 生成一个随机数，用于选择一个拾取类
			SpawnedPickup = World->SpawnActor<APickup>(PickupClasses[RandomIndex], GetActorTransform());	// 生成一个拾取

			if (HasAuthority() && SpawnedPickup)
			{
				SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);	// 当拾取被销毁时，重新开始生成拾取的定时器
			}

		}
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())	// 如果是服务器
	{
		SpawnPickup();	// 生成拾取
		// StartSpawnPickupTimer();	// 重新开始生成拾取的定时器
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestoryedActor)
{
	// 生成一个随机时间，用于生成拾取
	const float SpawnTime = FMath::RandRange(SpawnTimeMin, SpawnTimeMax);
	// 设置定时器，用于生成拾取
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnTime);
}

// Called every frame
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

