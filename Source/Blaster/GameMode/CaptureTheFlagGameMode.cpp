// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"

#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,
                                               ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// 夺旗模式下，我们不希望记录队伍的得分，所以我们不调用父类的PlayerEliminated函数
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	if (ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState))
	{
		bool bValidCapture = Flag->GetTeam() != Zone->Team;		// 有效的夺取
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
