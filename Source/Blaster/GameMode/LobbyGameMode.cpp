// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();		// ��ȡ�������

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

		check(MultiplayerSessionsSubsystem);

		if (NumberOfPlayers == MultiplayerSessionsSubsystem->GetDesiredNumPublicConnections())		// ����������Ϊ2
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;		// ʹ���޷�����

				FString MatchType = MultiplayerSessionsSubsystem->GetDesiredMatchType();		// ��ȡ��������
				if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));		// ���������� - ���������е�ָ���ĵ�ͼ
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/TeamsMap?listen"));		// ���������� - ���������е�ָ���ĵ�ͼ
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlagMap?listen"));		// ���������� - ���������е�ָ���ĵ�ͼ
				}

			}
		}
	}

	
}