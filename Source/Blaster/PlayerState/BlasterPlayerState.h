// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// 重写获取生命周期复制属性函数

	virtual void OnRep_Score() override;	// 重写得分复制属性函数

	UFUNCTION()
	virtual void OnRep_Defeats();	// 重写击败复制属性函数

	void AddToScore(float ScoreAmount);		// 增加得分
	void AddToDefeats(int32 DefeatsAmount);	// 增加击败数

private:
	UPROPERTY()
	class ABlasterCharacter* Character;		// 玩家角色

	UPROPERTY()
	class ABlasterPlayerController* Controller;	// 玩家控制器

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;	// 击败数

	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;		// 当前游戏的队伍

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	FORCEINLINE void SetTeam(ETeam NewTeam) { Team = NewTeam; }
};
