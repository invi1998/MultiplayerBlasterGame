// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
	DOREPLIFETIME(ABlasterGameState, GreenTeamScore);
	DOREPLIFETIME(ABlasterGameState, YellowTeamScore);
	DOREPLIFETIME(ABlasterGameState, PurpleTeamScore);
	DOREPLIFETIME(ABlasterGameState, OrangeTeamScore);
	DOREPLIFETIME(ABlasterGameState, PinkTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlackTeamScore);
	DOREPLIFETIME(ABlasterGameState, WhiteTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::RedTeamScored()
{
	++RedTeamScore;
}

void ABlasterGameState::BlueTeamScored()
{
	++BlueTeamScore;
}

void ABlasterGameState::OnRepRedTeamScore()
{
}

void ABlasterGameState::OnRepBlueTeamScore()
{
}

void ABlasterGameState::OnRepGreenTeamScore()
{
}

void ABlasterGameState::OnRepYellowTeamScore()
{
}

void ABlasterGameState::OnRepPurpleTeamScore()
{
}

void ABlasterGameState::OnRepOrangeTeamScore()
{
}

void ABlasterGameState::OnRepPinkTeamScore()
{
}

void ABlasterGameState::OnRepBlackTeamScore()
{
}

void ABlasterGameState::OnRepWhiteTeamScore()
{
}

