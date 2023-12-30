// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();		// 获取玩家数量
	if (NumberOfPlayers == 2)		// 如果玩家数量为2
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;		// 使用无缝旅行
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));		// 服务器旅行 - 服务器旅行到指定的地图
		}
	}
}