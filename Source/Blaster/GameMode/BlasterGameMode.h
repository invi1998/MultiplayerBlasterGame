// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	// ��������ʱ�䵽����ʾ��ʤ�߲���ʼ��ȴ��ʱ��
}

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaSeconds) override;

	// �����̭
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);	// ��������

	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;	// Ԥ��ʱ��

	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.f;		// ��Ϸ����ʱ��

	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.f;		// ��Ϸ������ȴʱ��

	float LevelStartingTime = 0.f;		// �ؿ���ʼʱ��

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;		// ����ʱʱ��

public:
	FORCEINLINE float GetCountdownTime() const { return CooldownTime; }	// ��ȡ����ʱʱ��

};
