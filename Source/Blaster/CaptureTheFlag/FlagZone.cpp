// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "Blaster/Weapon/Flag.h"
#include "Components/SphereComponent.h"
#include "Blaster/GameMode/CaptureTheFlagGameMode.h"
#include "Kismet/KismetSystemLibrary.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;		// 禁用Tick

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));		// 创建区域球体
	SetRootComponent(ZoneSphere);		// 设置根组件
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	UKismetSystemLibrary::PrintString(this, TEXT("FlagZone"), true, true, FLinearColor::Red, 5.f);		// 输出旗帜区域
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnStartOverlap);	// 添加开始重叠事件
}

void AFlagZone::OnStartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UKismetSystemLibrary::PrintString(this, TEXT("Overlap"), true, true, FLinearColor::Red, 5.f);		// 输出重叠
	if (AFlag* OverlappingFlag = Cast<AFlag>(OtherActor))		// 如果夺旗存在且队伍不同
	{
		if (OverlappingFlag->GetTeam() != Team)
		{
			if (ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>())
			{
				GameMode->FlagCaptured(OverlappingFlag, this);
			}
			OverlappingFlag->ResetFlag();	// 重置旗帜
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, TEXT("Same Team"), true, true, FLinearColor::Red, 5.f);		// 输出同队伍
		}
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Not Flag"), true, true, FLinearColor::Red, 5.f);		// 输出不是旗帜
	}
}

