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
	// 将其设置为true时，游戏模式将会停留在等待开始的状态，它会为所有玩家生成一个默认设置，他们可以使用它在地图中飞行，没有网格
	bDelayedStart = true;

}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 如果游戏状态为等待开始
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
			BlasterPlayer->OnMatchStateSet(MatchState, bTeamMatch);
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
				WinnerCharacter->MulticastGainedTheCrown();	// 获得王冠
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))	// 如果不包含
			{
				ABlasterCharacter* LoserCharacter = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (LoserCharacter)
				{
					LoserCharacter->MulticastLostTheCrown();	// 失去王冠
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

	// 通知所有玩家，谁击杀了谁
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
	// 销毁玩家实体
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	// 重设玩家的游戏状态
	if (ElimmedController)
	{
		// 他会在起始点位置生成一个新角色
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		// RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);

		// 推开周围的玩家

		// 先获取出生点的位置
		FVector PlayerStartLocation = PlayerStarts[Selection]->GetActorLocation();

		// 获取出点已经存在的玩家列表
		TArray<FOverlapResult> OverlappingActors;
		if (UWorld* World = GetWorld())
		{
			// 获取出生点周围的玩家
			World->OverlapMultiByObjectType(
				OverlappingActors, 
				PlayerStartLocation, 
				FQuat::Identity, 
				FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects),
				FCollisionShape::MakeSphere(100.f));

			// 遍历玩家列表，然后推开他们
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
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();	// 获取游戏状态
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn()))
	{
		BlasterCharacter->Elim(true);
	}
}

float ABlasterGameMode::CalculateDamage(AController* VictimController, AController* AttackerController, float Damage)
{
	//if (AttackPlayerState && AttackPlayerState != VictimPlayerState && BlasterGameState)
	//{
	//	TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
	//	for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
	//	{
	//		PlayersCurrentlyInTheLead.Add(LeadPlayer);
	//	}

	//	AttackPlayerState->AddToScore(1.f);
	//	BlasterGameState->UpdateTopScore(AttackPlayerState);

	//	if (BlasterGameState->TopScoringPlayers.Contains(AttackPlayerState))
	//	{
	//		ABlasterCharacter* WinnerCharacter = Cast<ABlasterCharacter>(AttackPlayerState->GetPawn());
	//		if (WinnerCharacter)
	//		{
	//			WinnerCharacter->MulticastGainedTheCrown();	// 获得王冠
	//		}
	//	}

	//	for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
	//	{
	//		if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))	// 如果不包含
	//		{
	//			ABlasterCharacter* LoserCharacter = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
	//			if (LoserCharacter)
	//			{
	//				LoserCharacter->MulticastLostTheCrown();	// 失去王冠
	//			}
	//		}
	//	}
	//}
	return Damage;
}

