// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"

#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,
                                               ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// ����ģʽ�£����ǲ�ϣ����¼����ĵ÷֣��������ǲ����ø����PlayerEliminated����
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	if (ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState))
	{
		bool bValidCapture = Flag->GetTeam() != Zone->Team;		// ��Ч�Ķ�ȡ
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeamScored();
		}
		else if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScored();
		}
	}

}
