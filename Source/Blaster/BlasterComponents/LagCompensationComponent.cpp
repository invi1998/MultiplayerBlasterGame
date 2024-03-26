// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character ? Character : Cast<ABlasterCharacter>(GetOwner());

	if (Character)
	{
		Package.Time = GetWorld()->TimeSeconds;
		Package.HitCharacter = Character;
		for(auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Extent = BoxPair.Value->GetScaledBoxExtent();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpolateFrame(const FFramePackage& OlderFrame,
	const FFramePackage& NewerFrame, float HitTime)
{
	FFramePackage InterpolatedFrame;
	const float Alpha = FMath::Clamp((HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time), 0.f, 1.f);	// 插值公式：(HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time)
	for(auto& BoxPair : OlderFrame.HitBoxInfo)
	{
		const FBoxInformation& OlderBoxInfo = BoxPair.Value;
		const FBoxInformation& NewerBoxInfo = NewerFrame.HitBoxInfo[BoxPair.Key];

		FBoxInformation InterpolatedBoxInfo;
		InterpolatedBoxInfo.Location = FMath::Lerp(OlderBoxInfo.Location, NewerBoxInfo.Location, Alpha);
		InterpolatedBoxInfo.Extent = FMath::Lerp(OlderBoxInfo.Extent, NewerBoxInfo.Extent, Alpha);
		InterpolatedBoxInfo.Rotation = FMath::Lerp(OlderBoxInfo.Rotation, NewerBoxInfo.Rotation, Alpha);

		InterpolatedFrame.HitBoxInfo.Add(BoxPair.Key, InterpolatedBoxInfo);
	}

	return InterpolatedFrame;
}

FServerSideRewindResult ULagCompensationComponent::CheckHit(const FFramePackage& FramePackage,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	FServerSideRewindResult Result;
	Result.bHitConfirmed = false;
	Result.bHeadShot = false;

	if (HitCharacter == nullptr) return Result;

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame);	// 缓存当前帧数据
	MoveBoxes(HitCharacter, FramePackage);	// 移动命中角色的命中框
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);	// 关闭角色的碰撞

	// 首先启用命中框的碰撞，然后进行射线检测
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);	// 设置碰撞响应

	FHitResult HitResult;	// 射线检测
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;	// 射线终点
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);	// 射线检测

		if (HitResult.bBlockingHit)		// 命中
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);	// 重置命中框
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// 启用角色的碰撞
			Result.bHitConfirmed = true;	// 命中确认
			Result.bHeadShot = true;	// 判断是否是爆头
		}
		else
		{
			for (auto& BoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (BoxPair.Value != nullptr)
				{
					BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	// 启用命中框的碰撞
					BoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);	// 设置碰撞响应
				}
			}

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);	// 射线检测
			if (HitResult.bBlockingHit)		// 命中
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);	// 重置命中框
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// 启用角色的碰撞
				Result.bHitConfirmed = true;	// 命中确认
				Result.bHeadShot = false;	// 判断是否是爆头
				return Result;
			}
		}
	}

	return Result;
}

void ULagCompensationComponent::CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for(auto& BoxPair : HitCharacter->HitCollisionBoxes)
	{
		FBoxInformation BoxInfo;
		BoxInfo.Location = BoxPair.Value->GetComponentLocation();
		BoxInfo.Extent = BoxPair.Value->GetScaledBoxExtent();
		BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
		OutFramePackage.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;

	for(auto& BoxPair : FramePackage.HitBoxInfo)		// 遍历帧数据
	{
		UBoxComponent* BoxComponent = HitCharacter->HitCollisionBoxes[BoxPair.Key];	// 获取命中框
		if (BoxComponent)
		{
			BoxComponent->SetWorldLocation(BoxPair.Value.Location);		// 设置命中框的位置
			BoxComponent->SetWorldRotation(BoxPair.Value.Rotation);		// 设置命中框的旋转
			BoxComponent->SetBoxExtent(BoxPair.Value.Extent);			// 设置命中框的大小
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& BoxPair : FramePackage.HitBoxInfo)		// 遍历帧数据
	{
		UBoxComponent* BoxComponent = HitCharacter->HitCollisionBoxes[BoxPair.Key];	// 获取命中框
		if (BoxComponent)
		{
			BoxComponent->SetWorldLocation(BoxPair.Value.Location);		// 设置命中框的位置
			BoxComponent->SetWorldRotation(BoxPair.Value.Rotation);		// 设置命中框的旋转
			BoxComponent->SetBoxExtent(BoxPair.Value.Extent);			// 设置命中框的大小
			BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 禁用命中框的碰撞
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type Collision)
{
	if (HitCharacter == nullptr || HitCharacter->GetMesh() == nullptr) return;

	HitCharacter->GetMesh()->SetCollisionEnabled(Collision);	// 设置角色的碰撞
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;		// 如果角色为空或者不是服务器，就直接返回

	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFramePackage;
		SaveFramePackage(ThisFramePackage);
		FrameHistory.AddHead(ThisFramePackage);
	}
	else
	{
		// 我们通过时间来判断是否需要保存帧数据
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;	// 获取最新的帧数据和最老的帧数据的时间差

		while (HistoryLength > MaxRecordTime)	// 如果时间差大于最大记录时间，就删除最老的帧数据
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());	// 删除最老的帧数据
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;	// 重新计算时间差
		}

		FFramePackage ThisFramePackage;
		SaveFramePackage(ThisFramePackage);
		FrameHistory.AddHead(ThisFramePackage);

		// ShowFramePackage(ThisFramePackage, FColor::Red);	// 显示当前帧数据
	}
}

// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();	// 保存帧数据
	
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	Character = Character ? Character : Cast<ABlasterCharacter>(GetOwner());

	if (Character)
	{
		for(auto& BoxPair : Package.HitBoxInfo)
		{
			const FBoxInformation BoxInfo = BoxPair.Value;
			FVector Location = BoxInfo.Location;
			FVector Extent = BoxInfo.Extent;
			FRotator Rotation = BoxInfo.Rotation;

			DrawDebugBox(GetWorld(), Location, Extent, Rotation.Quaternion(), Color, false, 5.0f);
		}
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	const FFramePackage RewindFramePackage = GetFrameToCheck(HitCharacter, HitTime);	// 获取需要检查的帧数据

	return CheckHit(RewindFramePackage, HitCharacter, TraceStart, HitLocation);	// 检查命中
}


FFramePackage ULagCompensationComponent::GetFrameToCheck(const ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.Num() == 0;

	if (bReturn) return FFramePackage();

	FFramePackage InterpolatedFrame{};
	bool bLerp = true;

	const TDoubleLinkedList<FFramePackage>& HistoryFrame = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHitTime = HistoryFrame.GetTail()->GetValue().Time;
	if (OldestHitTime > HitTime) return FFramePackage();

	if (OldestHitTime == HitTime)
	{
		// 如果最老的帧数据的时间等于命中时间，就直接获取最老的帧数据
		InterpolatedFrame = HistoryFrame.GetTail()->GetValue();
		bLerp = false;
	}

	const float NewestHitTime = HistoryFrame.GetHead()->GetValue().Time;
	if (NewestHitTime <= HitTime)
	{
		// 如果最新的帧数据的时间小于等于命中时间，存储最新的帧数据
		InterpolatedFrame = HistoryFrame.GetHead()->GetValue();
		bLerp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = HistoryFrame.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;

	// 循环遍历历史帧数据，找到命中时间对应的帧数据（命中时间在两帧数据之间，因为float精度问题，可能不会命中）OlderNode->GetValue().Time > HitTime && YoungerNode->GetValue().Time <= HitTime
	while (OlderNode->GetValue().Time > HitTime)
	{
		// next节点为空，表示到达最老的帧数据
		if (OlderNode->GetNextNode() == nullptr)
		{
			break;
		}

		if (OlderNode->GetValue().Time > HitTime)	// 如果次新的帧数据的时间大于命中时间
		{
			YoungerNode = OlderNode;	// 节点向前移动
		}
	}

	if (OlderNode->GetValue().Time == HitTime)	// 如果次新的帧数据的时间等于命中时间，就直接获取次新的帧数据
	{
		// 获取次新的帧数据和最新的帧数据
		InterpolatedFrame = OlderNode->GetValue();
		bLerp = false;
	}

	if (bLerp)
	{
		// 如果需要插值，就进行插值，插值公式：(HitTime - YoungerNode->GetValue().Time) / (OlderNode->GetValue().Time - YoungerNode->GetValue().Time)
		InterpolatedFrame = InterpolateFrame(YoungerNode->GetValue(), OlderNode->GetValue(), HitTime);
	}

	return InterpolatedFrame;
}

FServerSideRewindResult_Shotgun ULagCompensationComponent::ServerSideRewind_Shotgun(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FServerSideRewindResult_Shotgun Result{};

	TArray<FFramePackage> RewindFramePackages;		// 用于存储需要检查的帧数据

	for (const ABlasterCharacter* HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr) continue;

		const FFramePackage RewindFramePackage = GetFrameToCheck(HitCharacter, HitTime);	// 获取需要检查的帧数据
		RewindFramePackages.Add(RewindFramePackage);
	}



	return Result;
}

FServerSideRewindResult_Shotgun ULagCompensationComponent::CheckHit_Shotgun(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	FServerSideRewindResult_Shotgun Result{};

	if (FramePackages.Num() != HitLocations.Num()) return Result;

	

	return Result;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
                                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	if (HitCharacter == nullptr) return;

	FServerSideRewindResult Result = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);	// 服务器端倒带
	if (Result.bHitConfirmed && DamageCauser)
	{
		// 如果命中确认，就处理伤害
		UGameplayStatics::ApplyDamage(
			HitCharacter,	// 受击角色
			DamageCauser->GetDamage(), // 伤害值
			Character->Controller,	// 伤害来源
			DamageCauser,	// 伤害来源
			UDamageType::StaticClass()	// 伤害类型
			);
	}
}

