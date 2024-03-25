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
		// ����ͨ��ʱ�����ж��Ƿ���Ҫ����֡����
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;	// ��ȡ���µ�֡���ݺ����ϵ�֡���ݵ�ʱ���

		while (HistoryLength > MaxRecordTime)	// ���ʱ����������¼ʱ�䣬��ɾ�����ϵ�֡����
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());	// ɾ�����ϵ�֡����
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;	// ���¼���ʱ���
		}

		FFramePackage ThisFramePackage;
		SaveFramePackage(ThisFramePackage);
		FrameHistory.AddHead(ThisFramePackage);

		ShowFramePackage(ThisFramePackage, FColor::Red);	// ��ʾ��ǰ֡����
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

	FFramePackage RewindFramePackage;	// ���ڴ洢������֡����
	bool bLerp = true;	// �Ƿ��ֵ���������ʱ������֡����֮�䣬����Ҫ��ֵ���������ʱ�����ĳһ֡���ݵ�ʱ�䣬�Ͳ���Ҫ��ֵ

	// ��ȡ��ǰ�ܻ���ɫ����ʷ֡����
	const TDoubleLinkedList<FFramePackage>& FrameHistory = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHitTime = FrameHistory.GetTail()->GetValue().Time;	// ��ȡ���ϵ�֡���ݵ�ʱ��
	if (OldestHitTime > HitTime) return;	// ������ϵ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӷ��أ������˵���ʱ�䣩

	if (OldestHitTime == HitTime)	// ������ϵ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӻ�ȡ���ϵ�֡����
	{
		RewindFramePackage = FrameHistory.GetTail()->GetValue();
		bLerp = false;
	}

	const float NewestHitTime = FrameHistory.GetHead()->GetValue().Time;	// ��ȡ���µ�֡���ݵ�ʱ��
	if (NewestHitTime <= HitTime)
	{
		// ������µ�֡���ݵ�ʱ��С�ڵ�������ʱ�䣬�洢���µ�֡����
		RewindFramePackage = FrameHistory.GetHead()->GetValue();
		bLerp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = FrameHistory.GetHead();		// ��ȡ���µ�֡����
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;	// ��ȡ���µ�֡����

	// ѭ��������ʷ֡���ݣ��ҵ�����ʱ���Ӧ��֡���ݣ�����ʱ������֡����֮�䣬��Ϊfloat�������⣬���ܲ������У�OlderNode->GetValue().Time > HitTime && YoungerNode->GetValue().Time <= HitTime
	while (OlderNode->GetValue().Time > HitTime)	// ������µ�֡���ݵ�ʱ���������ʱ��
	{
		if (OlderNode->GetNextNode() == nullptr)	// ������µ�֡���ݵ�ǰһ���ڵ�Ϊ�գ���ֱ�ӷ���
		{
			break;
		}

		if (OlderNode->GetValue().Time > HitTime && YoungerNode->GetValue().Time <= HitTime)	// ������µ�֡���ݵ�ʱ���������ʱ�䣬�����µ�֡���ݵ�ʱ��С�ڵ�������ʱ��
		{
			// ��ȡ���µ�֡���ݺ����µ�֡����
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
		// �����Ҫ��ֵ���ͽ��в�ֵ����ֵ��ʽ��(HitTime - YoungerNode->GetValue().Time) / (OlderNode->GetValue().Time - YoungerNode->GetValue().Time)
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

