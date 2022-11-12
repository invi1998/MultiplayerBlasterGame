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
	// �洢 Blaster �ַ�����
	// ��Ҫ�Ӷ�����ͼ������������ϣ������Ϊ��ͼֻ��������ΪCharacter
	// ��Ϊ����һ��˽�б��������ǲ�����˽�б��������� BlueprintReadOnly �� ���Կ������� meta ��AllowPrivateAccess ����Ϊtrue
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		class ABlasterCharacter* BlasterCharacter;

	// �ٶ�
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Spead;

	// �Ƿ��ڿ���
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	// �Ƿ����, ������������
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;

	// ����Ƿ��е�ǰװ��������
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bWeaponEquipped;

	// ����Ƿ����
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouched;

	// �����׼
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bAiming;
};
