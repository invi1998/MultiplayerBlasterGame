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
	void SetHUDShield(float Shield, float MaxShield);	// ���û���
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime();

	virtual void ReceivedPlayer() override;		// ����Ҽ����ʱ�򣬾����������������ʱ��ͬ��

	void OnMatchStateSet(FName state);	// ����Ϸ״̬�����仯ʱ�����øú���


protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDTime();
	void PollInit();

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
		float TimeSyncFrequency = 5.f;	// ʱ��ͬ��Ƶ��

	float TimeSyncRunningTime = 0.f;	// ʱ��ͬ������ʱ��

	void CheckTimeSync(float DeltaTime);	// ���ʱ��ͬ��
	void HandleMatchHasStarted();	// ������Ϸ��ʼ
	void HandleCooldown();	// ������ȴʱ��

	// �����������Ϸ״̬
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();

	// �ͻ��˼�����Ϸ
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);	// �ͻ�������Ϸ��;����

private:
	UPROPERTY()
		class ABlasterHUD* BlasterHUD;	// HUD

	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;	// ��Ϸģʽ

	float LevelStartingTime = 0.f;	//	��Ϸ��ʼʱ��
	float MatchTime = 0.f;		// ��Ϸʱ��
	float WarmupTime = 0.f;		// Ԥ��ʱ��
	float CooldownTime = 0.f;	//	��ȴʱ��
	uint32 CountdownInt = 0;	// ����ʱ��������

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;	// ��ǰ��Ϸ״̬

	UFUNCTION()
		void OnRep_MatchState();	// ��MatchState�����仯ʱ�����øú���

	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;	// ��ɫ���ǲ�

	bool bInitializeCharacterOverlay = false;	// �Ƿ��ʼ����ɫ���ǲ�
	float HUDHealth;	// ����ֵ
	float HUDMaxHealth;	// �������ֵ
	float HUDShield;	// ����
	float HUDMaxShield;	// ��󻤶�
	float HUDScore;	 // ����
	int32 HUDDefeats;	// ��ɱ��
	int32 HUDGrenades;	// ��������
};
