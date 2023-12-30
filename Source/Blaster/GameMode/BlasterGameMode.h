// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	// 比赛持续时间到，显示获胜者并开始冷却计时器
}

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaSeconds) override;

	// 玩家淘汰
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);	// 请求重生

	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;	// 预热时间

	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.f;		// 游戏比赛时间

	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.f;		// 游戏比赛冷却时间

	float LevelStartingTime = 0.f;		// 关卡开始时间

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;		// 倒计时时间

public:
	FORCEINLINE float GetCountdownTime() const { return CooldownTime; }	// 获取倒计时时间

};
