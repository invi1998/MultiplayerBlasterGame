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
	TArray<ABlasterPlayerState*> RedTeam;		// ���
	TArray<ABlasterPlayerState*> BlueTeam;		// ����
	TArray<ABlasterPlayerState*> GreenTeam;		// �̶�
	TArray<ABlasterPlayerState*> YellowTeam;	// �ƶ�
	TArray<ABlasterPlayerState*> PurpleTeam;	// �϶�
	TArray<ABlasterPlayerState*> OrangeTeam;	// �ȶ�
	TArray<ABlasterPlayerState*> PinkTeam;		// �۶�
	TArray<ABlasterPlayerState*> BlackTeam;		// �ڶ�
	TArray<ABlasterPlayerState*> WhiteTeam;		// �׶�
	
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
