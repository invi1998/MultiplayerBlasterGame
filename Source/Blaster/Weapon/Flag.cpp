// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Blaster/Character/BlasterCharacter.h"
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

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	// 分离武器组件
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	// 清空武器所有者
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
	// 开启武器物理模拟，模拟掉落地面的弹跳
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 添加碰撞，设置对所有通道的碰撞响应为阻塞
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// 忽略pawon的碰撞
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

}
