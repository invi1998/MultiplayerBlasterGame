// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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

void AFlag::ResetFlag()
{
	if (ABlasterCharacter* Player = Cast<ABlasterCharacter>(GetOwner()))
	{
		Player->SetIsHoldingFlag(false);	// ������Ҳ���������
		Player->SetOverlappingWeapon(nullptr);	// ������Ҳ��ص�����
		Player->UnCrouch();	// վ��
	}

	SetActorTransform(InitialTransform);

	if (!HasAuthority()) return;

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);

	SetWeaponState(EWeaponState::EWS_Initial);

	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	// �����������忪����ײ
	GetAreaSphere()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);	// ���ö�Pawn����ײ��ӦΪ�ص�

	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// ������������ر���ײ
	FlagMesh->SetSimulatePhysics(false);		// �ر�FlagMesh������ģ��
	FlagMesh->SetEnableGravity(false);		// �ر�FlagMesh������
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);		// ����FlagMesh������ײ	
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);	// ���ö�WorldDynamic����ײ��ӦΪ�ص�

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
	UKismetSystemLibrary::PrintString(this, TEXT("Flag OnDropped"), true, true, FLinearColor::Red, 5.0f);
	EnableCustomDepth(true);

}
