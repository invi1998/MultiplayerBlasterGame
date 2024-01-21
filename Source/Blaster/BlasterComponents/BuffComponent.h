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

	friend class ABlasterCharacter;		// ����ABlasterCharacter����˽�г�Ա

	void AddHealth(float HealthAmount, float HealingTime);	// ����Ѫ�� ����Ѫ��������ʱ��

	void AddSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);		// �����ٶ� �����ٶȺͳ���ʱ��

	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);		// ���ó�ʼ�ٶ�

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);		// ������������, ����DeltaTime, ���ڼ�����������

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
	 * �ٶ�buff
	 */
	FTimerHandle SpeedBuffTimerHandle;		// �ٶ�buff��ʱ�����
	void ResetSpeed();		// �����ٶ�
	float InitialBaseSpeed = 0.0f;		// ��ʼ�����ٶ�
	float InitialCrouchSpeed = 0.0f;	// ��ʼ�����ٶ�

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);		// �ٶ�buff�ಥ����


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
