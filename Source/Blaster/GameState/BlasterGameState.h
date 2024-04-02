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
	TArray<ABlasterPlayerState*> RedTeam;		// 红队
	TArray<ABlasterPlayerState*> BlueTeam;		// 蓝队
	TArray<ABlasterPlayerState*> GreenTeam;		// 绿队
	TArray<ABlasterPlayerState*> YellowTeam;	// 黄队
	TArray<ABlasterPlayerState*> PurpleTeam;	// 紫队
	TArray<ABlasterPlayerState*> OrangeTeam;	// 橙队
	TArray<ABlasterPlayerState*> PinkTeam;		// 粉队
	TArray<ABlasterPlayerState*> BlackTeam;		// 黑队
	TArray<ABlasterPlayerState*> WhiteTeam;		// 白队
	
	TMap<ETeam, float> TeamScores;

	void RedTeamScored();
	void BlueTeamScored();

	UPROPERTY(ReplicatedUsing=OnRepRedTeamScore)
	float RedTeamScore = 0.f;

	UFUNCTION()
	void OnRepRedTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepBlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRepBlueTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepGreenTeamScore)
	float GreenTeamScore = 0.f;

	UFUNCTION()
	void OnRepGreenTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepYellowTeamScore)
	float YellowTeamScore = 0.f;

	UFUNCTION()
	void OnRepYellowTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepPurpleTeamScore)
	float PurpleTeamScore = 0.f;

	UFUNCTION()
	void OnRepPurpleTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepOrangeTeamScore)
	float OrangeTeamScore = 0.f;

	UFUNCTION()
	void OnRepOrangeTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepPinkTeamScore)
	float PinkTeamScore = 0.f;

	UFUNCTION()
	void OnRepPinkTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepBlackTeamScore)
	float BlackTeamScore = 0.f;

	UFUNCTION()
	void OnRepBlackTeamScore();

	UPROPERTY(ReplicatedUsing=OnRepWhiteTeamScore)
	float WhiteTeamScore = 0.f;

	UFUNCTION()
	void OnRepWhiteTeamScore();

private:
	float TopScore = 0.f;

};
