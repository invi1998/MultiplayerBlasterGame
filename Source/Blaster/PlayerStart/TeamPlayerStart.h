// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/BlasterTypes/Team.h"

#include "TeamPlayerStart.generated.h"

/**
 * ��ҳ����㣨���ֶ��飩
 */
UCLASS()
class BLASTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:

protected:
	UPROPERTY(EditAnywhere)
	ETeam Team;	// ����

private:

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
};
