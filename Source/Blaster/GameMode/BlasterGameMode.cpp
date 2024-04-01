// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Engine/PawnIterator.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	// ��������Ϊtrueʱ����Ϸģʽ����ͣ���ڵȴ���ʼ��״̬������Ϊ�����������һ��Ĭ�����ã����ǿ���ʹ�����ڵ�ͼ�з��У�û������
	bDelayedStart = true;

}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// �����Ϸ״̬Ϊ�ȴ���ʼ
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);

		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*Iterator);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
                                        ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ABlasterPlayerState* AttackPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackPlayerState && AttackPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackPlayerState);

		if (BlasterGameState->TopScoringPlayers.Contains(AttackPlayerState))
		{
			ABlasterCharacter* WinnerCharacter = Cast<ABlasterCharacter>(AttackPlayerState->GetPawn());
			if (WinnerCharacter)
			{
				WinnerCharacter->MulticastGainedTheCrown();	// �������
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))	// ���������
			{
				ABlasterCharacter* LoserCharacter = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (LoserCharacter)
				{
					LoserCharacter->MulticastLostTheCrown();	// ʧȥ����
				}
			}
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	// ֪ͨ������ң�˭��ɱ��˭
	if (AttackPlayerState && VictimPlayerState)
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*Iterator))
			{
				BlasterPlayer->BroadcastElimAnnouncement(AttackPlayerState, VictimPlayerState);
			}
		}
	}
	
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	// �������ʵ��
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	// ������ҵ���Ϸ״̬
	if (ElimmedController)
	{
		// ��������ʼ��λ������һ���½�ɫ
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		// RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);

		// �ƿ���Χ�����

		// �Ȼ�ȡ�������λ��
		FVector PlayerStartLocation = PlayerStarts[Selection]->GetActorLocation();

		// ��ȡ�����Ѿ����ڵ�����б�
		TArray<FOverlapResult> OverlappingActors;
		if (UWorld* World = GetWorld())
		{
			// ��ȡ��������Χ�����
			World->OverlapMultiByObjectType(
				OverlappingActors, 
				PlayerStartLocation, 
				FQuat::Identity, 
				FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects),
				FCollisionShape::MakeSphere(100.f));

			// ��������б�Ȼ���ƿ�����
			for (auto OverlappingActor : OverlappingActors)
			{
				if (ACharacter* OverlappingCharacter = Cast<ACharacter>(OverlappingActor.GetActor()))
				{
					FVector LaunchDirection = OverlappingCharacter->GetActorLocation() - PlayerStartLocation;
					LaunchDirection.Normalize();
					LaunchDirection.Z = 0.f;
					LaunchDirection *= 1000.f;
					OverlappingCharacter->LaunchCharacter(LaunchDirection, true, true);
				}
			}
		}

		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);

	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (!PlayerLeaving) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();	// ��ȡ��Ϸ״̬
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn()))
	{
		BlasterCharacter->Elim(true);
	}
}

