// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

public:
	ASpeedPickup();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;	// ��ײ���ص�ʱ����


private:
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.5f;	// �����ٶȼӳ�

	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 0.5f;	// �����ٶȼӳ�

	float SpeedBuffTime = 3.0f;	// �ٶȼӳɳ���ʱ��
};
