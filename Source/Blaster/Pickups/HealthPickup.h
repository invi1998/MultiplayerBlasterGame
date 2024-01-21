// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                             const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float HealthAmount = 100.0f;	// 血量

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.0f;	// 治疗时间，因为我们希望在拾取后有一个治疗的过程，而不是立刻就回满血量，所以我们需要一个治疗时间
};
