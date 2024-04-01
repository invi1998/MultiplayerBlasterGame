// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blaster/BlasterTypes/Team.h"

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> TopScoringPlayers;

	/*
	 * Team Scores
	 */
	//TArray<ABlasterPlayerState*> RedTeam;		// 红队
	//TArray<ABlasterPlayerState*> BlueTeam;		// 蓝队
	//TArray<ABlasterPlayerState*> GreenTeam;		// 绿队
	//TArray<ABlasterPlayerState*> YellowTeam;	// 黄队
	//TArray<ABlasterPlayerState*> PurpleTeam;	// 紫队
	//TArray<ABlasterPlayerState*> OrangeTeam;	// 橙队
	//TArray<ABlasterPlayerState*> PinkTeam;		// 粉队
	//TArray<ABlasterPlayerState*> BlackTeam;		// 黑队
	//TArray<ABlasterPlayerState*> WhiteTeam;		// 白队


	UPROPERTY(ReplicatedUsing=OnRepETeamScore)
	TMap<ETeam, float> TeamScores;

	UFUNCTION()
	void OnRepETeamScore(ETeam Team);

private:
	float TopScore = 0.f;

};
