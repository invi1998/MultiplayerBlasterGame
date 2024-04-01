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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// ��д��ȡ�������ڸ������Ժ���

	virtual void OnRep_Score() override;	// ��д�÷ָ������Ժ���

	UFUNCTION()
	virtual void OnRep_Defeats();	// ��д���ܸ������Ժ���

	void AddToScore(float ScoreAmount);		// ���ӵ÷�
	void AddToDefeats(int32 DefeatsAmount);	// ���ӻ�����

private:
	UPROPERTY()
	class ABlasterCharacter* Character;		// ��ҽ�ɫ

	UPROPERTY()
	class ABlasterPlayerController* Controller;	// ��ҿ�����

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;	// ������

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;		// ��ǰ��Ϸ�Ķ���

	UFUNCTION()
	void OnRep_Team();	// ��д���鸴�����Ժ���

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam NewTeam);
};
