// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * ����ǹ
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);

	void ShotgunTraceHitWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);		// ����ǹ��������ض������Ŀ��

private:
	// ����ǹÿ���ӵ������ɢ����
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	uint32 NumberOfPellets = 10;


	
};
