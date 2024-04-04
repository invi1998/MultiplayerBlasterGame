// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "Blaster/Weapon/Flag.h"
#include "Components/SphereComponent.h"
#include "Blaster/GameMode/CaptureTheFlagGameMode.h"
#include "Kismet/KismetSystemLibrary.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;		// ����Tick

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));		// ������������
	SetRootComponent(ZoneSphere);		// ���ø����
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnStartOverlap);	// ��ӿ�ʼ�ص��¼�
}

void AFlagZone::OnStartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFlag* OverlappingFlag = Cast<AFlag>(OtherActor))		// �����������Ҷ��鲻ͬ
	{
		if (OverlappingFlag->GetTeam() != Team)
		{
			if (ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>())
			{
				GameMode->FlagCaptured(OverlappingFlag, this);
			}
			OverlappingFlag->ResetFlag();	// ��������
		}
	}
}

