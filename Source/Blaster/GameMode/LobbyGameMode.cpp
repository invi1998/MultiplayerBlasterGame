// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();		// ��ȡ�������
	if (NumberOfPlayers == 2)		// ����������Ϊ2
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;		// ʹ���޷�����
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));		// ���������� - ���������е�ָ���ĵ�ͼ
		}
	}
}