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

FFramePackage ULagCompensationComponent::InterpolateFrame(const FFramePackage& OlderFrame,
	const FFramePackage& NewerFrame, float HitTime)
{
	FFramePackage InterpolatedFrame;
	const float Alpha = FMath::Clamp((HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time), 0.f, 1.f);	// ��ֵ��ʽ��(HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time)
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
	CacheBoxPosition(HitCharacter, CurrentFrame);	// ���浱ǰ֡����
	MoveBoxes(HitCharacter, FramePackage);	// �ƶ����н�ɫ�����п�
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);	// �رս�ɫ����ײ

	// �����������п����ײ��Ȼ��������߼��
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);	// ������ײ��Ӧ

	FHitResult HitResult;	// ���߼��
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;	// �����յ�
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);	// ���߼��

		if (HitResult.bBlockingHit)		// ����
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);	// �������п�
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// ���ý�ɫ����ײ
			Result.bHitConfirmed = true;	// ����ȷ��
			Result.bHeadShot = true;	// �ж��Ƿ��Ǳ�ͷ
		}
		else
		{
			for (auto& BoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (BoxPair.Value != nullptr)
				{
					BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	// �������п����ײ
					BoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);	// ������ײ��Ӧ
				}
			}

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);	// ���߼��
			if (HitResult.bBlockingHit)		// ����
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);	// �������п�
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// ���ý�ɫ����ײ
				Result.bHitConfirmed = true;	// ����ȷ��
				Result.bHeadShot = false;	// �ж��Ƿ��Ǳ�ͷ
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

	for(auto& BoxPair : FramePackage.HitBoxInfo)		// ����֡����
	{
		UBoxComponent* BoxComponent = HitCharacter->HitCollisionBoxes[BoxPair.Key];	// ��ȡ���п�
		if (BoxComponent)
		{
			BoxComponent->SetWorldLocation(BoxPair.Value.Location);		// �������п��λ��
			BoxComponent->SetWorldRotation(BoxPair.Value.Rotation);		// �������п����ת
			BoxComponent->SetBoxExtent(BoxPair.Value.Extent);			// �������п�Ĵ�С
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& BoxPair : FramePackage.HitBoxInfo)		// ����֡����
	{
		UBoxComponent* BoxComponent = HitCharacter->HitCollisionBoxes[BoxPair.Key];	// ��ȡ���п�
		if (BoxComponent)
		{
			BoxComponent->SetWorldLocation(BoxPair.Value.Location);		// �������п��λ��
			BoxComponent->SetWorldRotation(BoxPair.Value.Rotation);		// �������п����ת
			BoxComponent->SetBoxExtent(BoxPair.Value.Extent);			// �������п�Ĵ�С
			BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// �������п����ײ
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type Collision)
{
	if (HitCharacter == nullptr || HitCharacter->GetMesh() == nullptr) return;

	HitCharacter->GetMesh()->SetCollisionEnabled(Collision);	// ���ý�ɫ����ײ
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

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.Num() == 0;

	if (bReturn) return FServerSideRewindResult();

	FFramePackage RewindFramePackage;	// ���ڴ洢������֡����
	bool bLerp = true;	// �Ƿ��ֵ���������ʱ������֡����֮�䣬����Ҫ��ֵ���������ʱ�����ĳһ֡���ݵ�ʱ�䣬�Ͳ���Ҫ��ֵ

	// ��ȡ��ǰ�ܻ���ɫ����ʷ֡����
	const TDoubleLinkedList<FFramePackage>& HistoryFrame = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHitTime = HistoryFrame.GetTail()->GetValue().Time;	// ��ȡ���ϵ�֡���ݵ�ʱ��
	if (OldestHitTime > HitTime) return FServerSideRewindResult();	// ������ϵ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӷ��أ������˵���ʱ�䣩

	if (OldestHitTime == HitTime)	// ������ϵ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӻ�ȡ���ϵ�֡����
	{
		RewindFramePackage = HistoryFrame.GetTail()->GetValue();
		bLerp = false;
	}

	const float NewestHitTime = HistoryFrame.GetHead()->GetValue().Time;	// ��ȡ���µ�֡���ݵ�ʱ��
	if (NewestHitTime <= HitTime)
	{
		// ������µ�֡���ݵ�ʱ��С�ڵ�������ʱ�䣬�洢���µ�֡����
		RewindFramePackage = HistoryFrame.GetHead()->GetValue();
		bLerp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = HistoryFrame.GetHead();		// ��ȡ���µ�֡����
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
		RewindFramePackage = InterpolateFrame(YoungerNode->GetValue(), OlderNode->GetValue(), HitTime);
	}

	return CheckHit(RewindFramePackage, HitCharacter, TraceStart, HitLocation);	// �������
}

