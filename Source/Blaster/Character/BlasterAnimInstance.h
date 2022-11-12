// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	// 存储 Blaster 字符变量
	// 需要从动画蓝图访问它，并且希望设置为蓝图只读，分类为Character
	// 因为这是一个私有变量，我们不能在私有变量上设置 BlueprintReadOnly ， 所以可以设置 meta ，AllowPrivateAccess 设置为true
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		class ABlasterCharacter* BlasterCharacter;

	// 速度
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Spead;

	// 是否在空中
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	// 是否加速, 用于驱动动画
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;

	// 检测是否有当前装备的武器
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bWeaponEquipped;

	// 检测是否蹲下
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouched;

	// 检测瞄准
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bAiming;
};
