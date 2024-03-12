// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Pickups/HealthPickup.h"
#include "Blaster/Pickups/ShieldPickup.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();

	friend class ABlasterCharacter;		// ����ABlasterCharacter����˽�г�Ա

	void AddHealth(float HealthAmount, float HealingTime);	// ����Ѫ�� ����Ѫ��������ʱ��

	void AddSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);		// �����ٶ� �����ٶȺͳ���ʱ��

	void AddJump(float JumpZVelocity, float JumpTime);		// ������Ծ�ٶ� ������Ծ�ٶȺͳ���ʱ��

	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);		// ���ó�ʼ�ٶ�

	void SetInitialJumpZVelocity(float JumpZVelocity);		// ���ó�ʼ��Ծ�ٶ�
	void ReplenishShield(float ShieldReplenishAmount, float ShieldReplenishTime);		// ���ӻ��� ���뻤�ܲ������ͻ��ܲ���ʱ��

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);		// ������������, ����DeltaTime, ���ڼ�����������

	void ShieldRampUp(float DeltaTime);		// ������������, ����DeltaTime, ���ڼ��㻤������

private:
	UPROPERTY()
	class ABlasterCharacter* Character;		// ָ��ABlasterCharacter��ָ��

	/*
	 * ��Ѫbuff
	 */
	bool bHealing = false;		// �Ƿ���������
	float HealingRate = 0.0f;	// ��������
	float AmountToHeal = 0.0f;	// ������

	/*
	 * ����Buff
	 */
	bool bShieldReplenishing = false;			// �Ƿ����ڲ��令��
	float ShieldReplenishRate = 0.0f;			// ���ܲ�������
	float ShieldAmountToReplenish = 0.0f;		// ���ܲ�����

	/*
	 * �ٶ�buff
	 */
	FTimerHandle SpeedBuffTimerHandle;		// �ٶ�buff��ʱ�����
	void ResetSpeed();		// �����ٶ�
	float InitialBaseSpeed = 0.0f;		// ��ʼ�����ٶ�
	float InitialCrouchSpeed = 0.0f;	// ��ʼ�����ٶ�

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);		// �ٶ�buff�ಥ����

	/*
	 * ��Ծbuff
	 */
	FTimerHandle JumpBuffTimerHandle;		// ��Ծbuff��ʱ�����

	void RestJump();		// ������Ծ
	float InitialJumpZVelocity = 0.0f;		// ��ʼ��Ծ�ٶ�

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpZVelocity, float JumpTime);		// ��Ծbuff�ಥ����


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
