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
	virtual void PostLogin(APlayerController* NewPlayer) override;	// 在玩家登录时调用

	virtual void Logout(AController* Exiting) override;				// 在玩家退出时调用

protected:
	virtual void HandleMatchHasStarted() override;	// 在比赛开始时调用
	
};
