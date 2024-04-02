// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);		// ����FlagMeshΪ�����
	GetAreaSphere()->SetupAttachment(FlagMesh);		// ��AreaSphere���ӵ�FlagMesh��

	GetPickupWidget()->SetupAttachment(FlagMesh);	// ��ʰȡ��ʾ������ӵ�FlagMesh��

	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);	// ����FlagMesh����������ײ
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// ����FlagMesh��������ײ
}
