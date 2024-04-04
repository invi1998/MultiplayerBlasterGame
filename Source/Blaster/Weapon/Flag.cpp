// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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

void AFlag::ResetFlag()
{
	if (ABlasterCharacter* Player = Cast<ABlasterCharacter>(GetOwner()))
	{
		Player->SetIsHoldingFlag(false);	// 设置玩家不持有旗帜
		Player->SetOverlappingWeapon(nullptr);	// 设置玩家不重叠武器
		Player->UnCrouch();	// 站立
	}

	if (!HasAuthority()) return;

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);

	SetWeaponState(EWeaponState::EWS_Initial);

	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	// 设置区域球体开启碰撞
	GetAreaSphere()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);	// 设置对Pawn的碰撞响应为重叠

	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

	SetActorTransform(InitialTransform);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 设置区域球体关闭碰撞
	FlagMesh->SetSimulatePhysics(false);		// 关闭FlagMesh的物理模拟
	FlagMesh->SetEnableGravity(false);		// 关闭FlagMesh的重力
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);	// 设置FlagMesh的碰撞为查询
	FlagMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);	// 设置对动态物体的碰撞响应为重叠

	UKismetSystemLibrary::PrintString(this, TEXT("Flag OnEquipped"), true, true, FLinearColor::Red, 5.0f);
	
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
