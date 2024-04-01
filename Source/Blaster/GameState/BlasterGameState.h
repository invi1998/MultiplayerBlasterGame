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
	//TArray<ABlasterPlayerState*> RedTeam;		// ���
	//TArray<ABlasterPlayerState*> BlueTeam;		// ����
	//TArray<ABlasterPlayerState*> GreenTeam;		// �̶�
	//TArray<ABlasterPlayerState*> YellowTeam;	// �ƶ�
	//TArray<ABlasterPlayerState*> PurpleTeam;	// �϶�
	//TArray<ABlasterPlayerState*> OrangeTeam;	// �ȶ�
	//TArray<ABlasterPlayerState*> PinkTeam;		// �۶�
	//TArray<ABlasterPlayerState*> BlackTeam;		// �ڶ�
	//TArray<ABlasterPlayerState*> WhiteTeam;		// �׶�


	UPROPERTY(ReplicatedUsing=OnRepETeamScore)
	TMap<ETeam, float> TeamScores;

	UFUNCTION()
	void OnRepETeamScore(ETeam Team);

private:
	float TopScore = 0.f;

};
