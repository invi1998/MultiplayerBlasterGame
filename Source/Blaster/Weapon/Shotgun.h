// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 霰弹枪
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

	void ShotgunTraceHitWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets);		// 霰弹枪射击，返回多个命中目标

private:
	// 霰弹枪每发子弹射出的散弹数
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	uint32 NumberOfPellets = 10;


	
};
