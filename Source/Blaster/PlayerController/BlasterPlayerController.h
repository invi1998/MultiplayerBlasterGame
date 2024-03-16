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
	void SetHUDHealth(float Health, float MaxHealth);	// 设置生命值
	void SetHUDHealthNative(float Health, float MaxHealth);	// 设置生命值
	void SetHUDShield(float Shield, float MaxShield);	// 设置护盾
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

	virtual void ReceivedPlayer() override;		// 在玩家加入的时候，尽快与与服务器进行时间同步

	void OnMatchStateSet(FName state);	// 当游戏状态发生变化时，调用该函数


protected:
	virtual void BeginPlay() override;	// 当游戏开始时，调用该函数
	virtual void OnPossess(APawn* InPawn) override;		// 当玩家控制器开始控制一个Pawn时，调用该函数
	void SetHUDTime();	// 设置HUD时间
	void PollInit();	// 轮询初始化

	/*
	 * 服务端和客户端之间的同步时间
	 */
	// 请求当前服务器的时间，传入客户端发送请求的时间
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// 服务器将客户端的请求时间和服务器当前时间发回给客户端
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// 服务端和客户端之间的同步时间差
	float ClientServerDelta = 0.0f;

	// 客户端与服务器同步时间的间隔（频率）
	UPROPERTY(EditAnywhere, Category = Time)
		float TimeSyncFrequency = 5.f;	// 时间同步频率

	float TimeSyncRunningTime = 0.f;	// 时间同步运行时间

	void CheckTimeSync(float DeltaTime);	// 检查时间同步
	void HandleMatchHasStarted();	// 处理游戏开始
	void HandleCooldown();	// 处理冷却时间

	// 服务器检查游戏状态
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();

	// 客户端加入游戏
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);	// 客户端在游戏中途加入

private:
	UPROPERTY()
		class ABlasterHUD* BlasterHUD;	// HUD

	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;	// 游戏模式

	float LevelStartingTime = 0.f;	//	游戏开始时间
	float MatchTime = 0.f;		// 游戏时间
	float WarmupTime = 0.f;		// 预热时间
	float CooldownTime = 0.f;	//	冷却时间
	uint32 CountdownInt = 0;	// 倒计时整数部分

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;	// 当前游戏状态

	UFUNCTION()
		void OnRep_MatchState();	// 当MatchState发生变化时，调用该函数

	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;	// 角色覆盖层
		
	float HUDHealth;	// 生命值
	bool bInitializeHealth = false;	// 是否初始化生命值
	float HUDMaxHealth;	// 最大生命值
	float HUDShield;	// 护盾
	bool bInitializeShield = false;	// 是否初始化护盾
	float HUDMaxShield;	// 最大护盾
	float HUDScore;	 // 分数
	bool bInitializeScore = false;	// 是否初始化分数
	int32 HUDDefeats;	// 击杀数
	bool bInitializeDefeats = false;	// 是否初始化击杀数
	int32 HUDGrenades;	// 手榴弹数量
	bool bInitializeGrenades = false;	// 是否初始化手榴弹数量
	float HUDCarriedAmmo;	// 携带弹药
	bool bInitializeCarriedAmmo = false;	// 是否初始化携带弹药
	float HUDWeaponAmmo;	// 武器弹药
	bool bInitializeWeaponAmmo = false;	// 是否初始化武器弹药
};
