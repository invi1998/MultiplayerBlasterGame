// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "BacktrackingPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABacktrackingPickup : public APickup
{
	GENERATED_BODY()

public:
	ABacktrackingPickup();

private:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float BacktrackingTime = 5.f;	// 回溯时间

	UPROPERTY(EditAnywhere)
	float BacktrackingCostTime = 1.f;	// 回溯消耗时间

	UPROPERTY(EditAnywhere)
	float BuffAliveTime = 10.f;		// buff存活时间
};
