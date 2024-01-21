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
	float HealthAmount = 100.0f;	// Ѫ��

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.0f;	// ����ʱ�䣬��Ϊ����ϣ����ʰȡ����һ�����ƵĹ��̣����������̾ͻ���Ѫ��������������Ҫһ������ʱ��
};
