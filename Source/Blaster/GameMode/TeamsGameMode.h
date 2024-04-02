// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;	// 在玩家登录时调用
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController) override;	// 玩家淘汰

	virtual void Logout(AController* Exiting) override;				// 在玩家退出时调用

	virtual float CalculateDamage(AController* VictimController, AController* AttackerController, float Damage) override;	// 计算伤害

protected:
	virtual void HandleMatchHasStarted() override;	// 在比赛开始时调用
	
};
