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
	virtual void PostLogin(APlayerController* NewPlayer) override;	// ����ҵ�¼ʱ����

	virtual void Logout(AController* Exiting) override;				// ������˳�ʱ����

protected:
	virtual void HandleMatchHasStarted() override;	// �ڱ�����ʼʱ����
	
};
