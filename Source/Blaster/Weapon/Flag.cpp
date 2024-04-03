// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Blaster/Character/BlasterCharacter.h"
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

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	// �����������
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	// �������������
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

	if (bDestroyOnDrop)
	{
		Destroy();
	}
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	EnableCustomDepth(false);

}

void AFlag::OnDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	// ������������ģ�⣬ģ��������ĵ���
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// �����ײ�����ö�����ͨ������ײ��ӦΪ����
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// ����pawon����ײ
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

}
