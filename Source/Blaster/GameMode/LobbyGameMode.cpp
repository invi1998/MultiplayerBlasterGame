// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();		// 获取玩家数量

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

		check(MultiplayerSessionsSubsystem);

		if (NumberOfPlayers == MultiplayerSessionsSubsystem->GetDesiredNumPublicConnections())		// 如果玩家数量为2
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;		// 使用无缝旅行

				FString MatchType = MultiplayerSessionsSubsystem->GetDesiredMatchType();		// 获取比赛类型
				if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));		// 服务器旅行 - 服务器旅行到指定的地图
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/TeamsMap?listen"));		// 服务器旅行 - 服务器旅行到指定的地图
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlagMap?listen"));		// 服务器旅行 - 服务器旅行到指定的地图
				}

			}
		}
	}

	
}