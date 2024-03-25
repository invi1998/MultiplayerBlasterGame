// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"

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


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

		ShowFramePackage(ThisFramePackage, FColor::Red);	// 显示当前帧数据
	}
	
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

void ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.Num() == 0;

	FFramePackage RewindFramePackage;	// 用于存储倒带的帧数据
	bool bLerp = true;	// 是否插值，如果命中时间在两帧数据之间，就需要插值，如果命中时间等于某一帧数据的时间，就不需要插值

	// 获取当前受击角色的历史帧数据
	const TDoubleLinkedList<FFramePackage>& FrameHistory = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHitTime = FrameHistory.GetTail()->GetValue().Time;	// 获取最老的帧数据的时间
	if (OldestHitTime > HitTime) return;	// 如果最老的帧数据的时间大于命中时间，就直接返回（超出了倒带时间）

	if (OldestHitTime == HitTime)	// 如果最老的帧数据的时间等于命中时间，就直接获取最老的帧数据
	{
		RewindFramePackage = FrameHistory.GetTail()->GetValue();
		bLerp = false;
	}

	const float NewestHitTime = FrameHistory.GetHead()->GetValue().Time;	// 获取最新的帧数据的时间
	if (NewestHitTime <= HitTime)
	{
		// 如果最新的帧数据的时间小于等于命中时间，存储最新的帧数据
		RewindFramePackage = FrameHistory.GetHead()->GetValue();
		bLerp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = FrameHistory.GetHead();		// 获取最新的帧数据
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;	// 获取次新的帧数据

	// 循环遍历历史帧数据，找到命中时间对应的帧数据（命中时间在两帧数据之间，因为float精度问题，可能不会命中）OlderNode->GetValue().Time > HitTime && YoungerNode->GetValue().Time <= HitTime
	while (OlderNode->GetValue().Time > HitTime)	// 如果次新的帧数据的时间大于命中时间
	{
		if (OlderNode->GetNextNode() == nullptr)	// 如果次新的帧数据的前一个节点为空，就直接返回
		{
			break;
		}

		if (OlderNode->GetValue().Time > HitTime && YoungerNode->GetValue().Time <= HitTime)	// 如果次新的帧数据的时间大于命中时间，且最新的帧数据的时间小于等于命中时间
		{
			// 获取次新的帧数据和最新的帧数据
			YoungerNode = OlderNode;
		}
	}
	if (OlderNode->GetValue().Time == HitTime)
	{
		RewindFramePackage = OlderNode->GetValue();
		bLerp = false;
	}

	if (bLerp)
	{
		// 如果需要插值，就进行插值，插值公式：(HitTime - YoungerNode->GetValue().Time) / (OlderNode->GetValue().Time - YoungerNode->GetValue().Time)
		const float Alpha = (HitTime - YoungerNode->GetValue().Time) / (OlderNode->GetValue().Time - YoungerNode->GetValue().Time);
		for(auto& BoxPair : YoungerNode->GetValue().HitBoxInfo)
		{
			const FBoxInformation& YoungerBoxInfo = BoxPair.Value;
			const FBoxInformation& OlderBoxInfo = OlderNode->GetValue().HitBoxInfo[BoxPair.Key];

			FBoxInformation RewindBoxInfo;
			RewindBoxInfo.Location = FMath::Lerp(YoungerBoxInfo.Location, OlderBoxInfo.Location, Alpha);
			RewindBoxInfo.Extent = FMath::Lerp(YoungerBoxInfo.Extent, OlderBoxInfo.Extent, Alpha);
			RewindBoxInfo.Rotation = FMath::Lerp(YoungerBoxInfo.Rotation, OlderBoxInfo.Rotation, Alpha);

			RewindFramePackage.HitBoxInfo.Add(BoxPair.Key, RewindBoxInfo);
		}
	}

	if (bReturn) return;


}

