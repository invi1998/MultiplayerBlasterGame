// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);		// 设置FlagMesh为根组件
	GetAreaSphere()->SetupAttachment(FlagMesh);		// 将AreaSphere连接到FlagMesh上

	GetPickupWidget()->SetupAttachment(FlagMesh);	// 将拾取提示组件连接到FlagMesh上

	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);	// 设置FlagMesh忽略所有碰撞
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 设置FlagMesh不开启碰撞
}
