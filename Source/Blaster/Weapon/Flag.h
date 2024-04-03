// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 旗帜
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

	virtual void Dropped() override;	// 重写丢弃方法

protected:
	virtual void OnEquipped() override;	// 重写装备时的方法
	virtual void OnDropped() override;	// 重写丢弃时的方法

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* FlagMesh;	// 旗帜模型
};
