// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

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

	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;	// Ԥ��ʱ��

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	float CountdownTime = 0.f;		// ����ʱʱ��

};
