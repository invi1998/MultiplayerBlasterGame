// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Pickups/HealthPickup.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();

	friend class ABlasterCharacter;		// 允许ABlasterCharacter访问私有成员

	void AddHealth(float HealthAmount, float HealingTime);	// 增加血量 传入血量和治疗时间

	void AddSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);		// 增加速度 传入速度和持续时间

	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);		// 设置初始速度

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);		// 治疗速率上升, 传入DeltaTime, 用于计算治疗速率

private:
	UPROPERTY()
	class ABlasterCharacter* Character;		// 指向ABlasterCharacter的指针

	/*
	 * 回血buff
	 */
	bool bHealing = false;		// 是否正在治疗
	float HealingRate = 0.0f;	// 治疗速率
	float AmountToHeal = 0.0f;	// 治疗量

	/*
	 * 速度buff
	 */
	FTimerHandle SpeedBuffTimerHandle;		// 速度buff计时器句柄
	void ResetSpeed();		// 重置速度
	float InitialBaseSpeed = 0.0f;		// 初始基础速度
	float InitialCrouchSpeed = 0.0f;	// 初始蹲下速度

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);		// 速度buff多播函数


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
