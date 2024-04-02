// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	// ��������Ϊtrueʱ����Ϸģʽ����ͣ���ڵȴ���ʼ��״̬������Ϊ�����������һ��Ĭ�����ã����ǿ���ʹ�����ڵ�ͼ�з��У�û������
	bDelayedStart = true;

	bTeamMatch = true;	// ����Ϊtrueʱ����Ϸģʽ�������Ŷ�֮����б���
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BlasterGameState)
	{
		ABlasterPlayerState* BlasterPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState && BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BlasterGameState->RedTeam.Num() <= BlasterGameState->BlueTeam.Num())
			{
				BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
				BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
			}
			else
			{
				BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
				BlasterGameState->BlueTeam.AddUnique(BlasterPlayerState);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* BlasterPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();

	if (BlasterGameState && BlasterPlayerState)
	{
		if (BlasterPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeam.Remove(BlasterPlayerState);
		}
		else if (BlasterPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeam.Remove(BlasterPlayerState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* VictimController, AController* AttackerController, float Damage)
{
	// ����ܺ��ߺ͹����߶���ͬһ���飬��ô�Ͳ�������˺�
	if (VictimController && AttackerController)
	{
		const ABlasterPlayerState* VictimPlayerState = VictimController->GetPlayerState<ABlasterPlayerState>();
		const ABlasterPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<ABlasterPlayerState>();

		if (VictimPlayerState && AttackerPlayerState && VictimPlayerState->GetTeam() == AttackerPlayerState->GetTeam())
		{
			return 0.f;
		}
	}

	return Damage;
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BlasterGameState)
	{
		for (APlayerState* PlayerState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerState);
			if (BlasterPlayerState && BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BlasterGameState->RedTeam.Num() <= BlasterGameState->BlueTeam.Num())
				{
					BlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
					BlasterGameState->RedTeam.AddUnique(BlasterPlayerState);
				}
				else
				{
					BlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
					BlasterGameState->BlackTeam.AddUnique(BlasterPlayerState);
				}
			}
		}
	}
}
