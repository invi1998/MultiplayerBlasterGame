// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void Tick(float DeltaSeconds) override;

	virtual float GetServerTime();

	virtual void ReceivedPlayer() override;		// ����Ҽ����ʱ�򣬾����������������ʱ��ͬ��

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDTime();

	/*
	 * ����˺Ϳͻ���֮���ͬ��ʱ��
	 */
	// ����ǰ��������ʱ�䣬����ͻ��˷��������ʱ��
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// ���������ͻ��˵�����ʱ��ͷ�������ǰʱ�䷢�ظ��ͻ���
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// ����˺Ϳͻ���֮���ͬ��ʱ���
	float ClientServerDelta = 0.0f;

	// �ͻ����������ͬ��ʱ��ļ����Ƶ�ʣ�
	UPROPERTY(EditAnywhere, Category = Time)
		float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

private:
	UPROPERTY()
		class ABlasterHUD* BlasterHUD;

	float MathTime = 128.f;
	uint32 CountdownInt = 0;
};
