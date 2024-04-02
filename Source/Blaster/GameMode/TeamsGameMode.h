// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;	// ����ҵ�¼ʱ����
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController) override;	// �����̭

	virtual void Logout(AController* Exiting) override;				// ������˳�ʱ����

	virtual float CalculateDamage(AController* VictimController, AController* AttackerController, float Damage) override;	// �����˺�

protected:
	virtual void HandleMatchHasStarted() override;	// �ڱ�����ʼʱ����
	
};
