// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * ÆìÖÄ
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* FlagMesh;	// ÆìÖÄÄ£ÐÍ
};
