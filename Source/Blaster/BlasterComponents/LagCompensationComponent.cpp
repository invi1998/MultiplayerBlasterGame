// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Blaster/Blaster.h"

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

// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();	// ����֡����

}

FFramePackage ULagCompensationComponent::InterpolateFrame(const FFramePackage& OlderFrame,
	const FFramePackage& NewerFrame, float HitTime) const
{
	FFramePackage InterpolatedFrame{};
	const float Alpha = FMath::Clamp((HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time), 0.f, 1.f);	// ��ֵ��ʽ��(HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time)
	for(auto& BoxPair : OlderFrame.HitBoxInfo)
	{
		const FBoxInformation& OlderBoxInfo = BoxPair.Value;
		const FBoxInformation& NewerBoxInfo = NewerFrame.HitBoxInfo[BoxPair.Key];

		FBoxInformation InterpolatedBoxInfo;
		InterpolatedBoxInfo.Location = FMath::Lerp(OlderBoxInfo.Location, NewerBoxInfo.Location, Alpha);
		InterpolatedBoxInfo.Extent = NewerBoxInfo.Extent;		// ����ǧ���ܲ�ֵ������ᵼ�����п��С�仯����Ϊ���п��С�ǹ̶��ģ�
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
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// ������ײ��Ӧ

	FHitResult HitResult;	// ���߼��
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;	// �����յ�
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);	// ���߼��

		if (HitResult.bBlockingHit)		// ����
		{
			if (HitResult.Component.IsValid())
			{
				if (UBoxComponent* HitBox = Cast<UBoxComponent>(HitResult.Component.Get()))
				{
					DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation().Quaternion(), FColor::Red, false, 5.0f);
				}
			}

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
					BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// ������ײ��Ӧ
				}
			}

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);	// ���߼��
			if (HitResult.bBlockingHit)		// ����
			{
				if (HitResult.Component.IsValid())
				{
					if (UBoxComponent* HitBox = Cast<UBoxComponent>(HitResult.Component.Get()))
					{
						DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation().Quaternion(), FColor::Blue, false, 5.0f);
					}
				}

				ResetHitBoxes(HitCharacter, CurrentFrame);	// �������п�
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// ���ý�ɫ����ײ
				Result.bHitConfirmed = true;	// ����ȷ��
				Result.bHeadShot = false;	// �ж��Ƿ��Ǳ�ͷ
				return Result;
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);	// �������п�
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// ���ý�ɫ����ײ

	return Result;
}

void ULagCompensationComponent::CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// �������п�
		if (HitBoxPair.Value != nullptr)	// ������п�Ϊ��
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();	// ��ȡ���п��λ��
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();	// ��ȡ���п����ת
			BoxInfo.Extent = HitBoxPair.Value->GetScaledBoxExtent();		// ��ȡ���п�Ĵ�С
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);

			DrawDebugBox(GetWorld(), BoxInfo.Location, BoxInfo.Extent, BoxInfo.Rotation.Quaternion(), FColor::Orange, false, 5.0f);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)	// �������п�
	{
		if (HitBoxPair.Value != nullptr)	// ������п�Ϊ��
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfo[HitBoxPair.Key].Location);	// �������п��λ��
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfo[HitBoxPair.Key].Rotation);	// �������п����ת
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfo[HitBoxPair.Key].Extent);			// �������п�Ĵ�С

			DrawDebugBox(GetWorld(), FramePackage.HitBoxInfo[HitBoxPair.Key].Location, FramePackage.HitBoxInfo[HitBoxPair.Key].Extent, FramePackage.HitBoxInfo[HitBoxPair.Key].Rotation.Quaternion(), FColor::Green, false, 5.0f);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfo[HitBoxPair.Key].Extent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			DrawDebugBox(GetWorld(), FramePackage.HitBoxInfo[HitBoxPair.Key].Location, FramePackage.HitBoxInfo[HitBoxPair.Key].Extent, FramePackage.HitBoxInfo[HitBoxPair.Key].Rotation.Quaternion(), FColor::Red, false, 5.0f);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type Collision)
{
	if (HitCharacter == nullptr || HitCharacter->GetMesh() == nullptr) return;

	HitCharacter->GetMesh()->SetCollisionEnabled(Collision);	// ���ý�ɫ����ײ
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;		// �����ɫΪ�ջ��߲��Ƿ���������ֱ�ӷ���

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

		// ShowFramePackage(ThisFramePackage, FColor::Red);	// ��ʾ��ǰ֡����
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
	const FFramePackage RewindFramePackage = GetFrameToCheck(HitCharacter, HitTime);	// ��ȡ��Ҫ����֡����

	return CheckHit(RewindFramePackage, HitCharacter, TraceStart, HitLocation);	// �������
}


FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.Num() == 0;

	if (bReturn)
	{
		// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("GetFrameToCheck bReturn: %d"), bReturn), true, false, FLinearColor::Red, 5.f);
		return FFramePackage();
	}

	FFramePackage InterpolatedFrame{};
	bool bLerp = true;

	const TDoubleLinkedList<FFramePackage>& HistoryFrame = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHitTime = HistoryFrame.GetTail()->GetValue().Time;
	if (OldestHitTime > HitTime)
	{
		// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("GetFrameToCheck OldestHitTime: %f, HitTime: %f"), OldestHitTime, HitTime), true, false, FLinearColor::Red, 5.f);
		return FFramePackage();	// ������ϵ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӷ��ؿ�
	}

	if (OldestHitTime == HitTime)
	{
		// ������ϵ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӻ�ȡ���ϵ�֡����
		InterpolatedFrame = HistoryFrame.GetTail()->GetValue();
		bLerp = false;
	}

	const float NewestHitTime = HistoryFrame.GetHead()->GetValue().Time;
	if (NewestHitTime <= HitTime)
	{
		// ������µ�֡���ݵ�ʱ��С�ڵ�������ʱ�䣬�洢���µ�֡����
		InterpolatedFrame = HistoryFrame.GetHead()->GetValue();
		bLerp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = HistoryFrame.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;

	// ѭ��������ʷ֡���ݣ��ҵ�����ʱ���Ӧ��֡���ݣ�����ʱ������֡����֮�䣬��Ϊfloat�������⣬���ܲ������У�OlderNode->GetValue().Time > HitTime && YoungerNode->GetValue().Time <= HitTime
	while (OlderNode->GetValue().Time > HitTime)
	{
		// next�ڵ�Ϊ�գ���ʾ�������ϵ�֡����
		if (OlderNode->GetNextNode() == nullptr)
		{
			break;
		}

		OlderNode = OlderNode->GetNextNode();

		if (OlderNode->GetValue().Time > HitTime)	// ������µ�֡���ݵ�ʱ���������ʱ��
		{
			YoungerNode = OlderNode;	// �ڵ���ǰ�ƶ�
		}
	}

	if (OlderNode->GetValue().Time == HitTime)	// ������µ�֡���ݵ�ʱ���������ʱ�䣬��ֱ�ӻ�ȡ���µ�֡����
	{
		// ��ȡ���µ�֡���ݺ����µ�֡����
		InterpolatedFrame = OlderNode->GetValue();
		bLerp = false;
	}

	if (bLerp)
	{
		// �����Ҫ��ֵ���ͽ��в�ֵ����ֵ��ʽ��(HitTime - YoungerNode->GetValue().Time) / (OlderNode->GetValue().Time - YoungerNode->GetValue().Time)
		InterpolatedFrame = InterpolateFrame(YoungerNode->GetValue(), OlderNode->GetValue(), HitTime);
	}

	InterpolatedFrame.HitCharacter = HitCharacter;

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("GetFrameToCheck bLerp: %d"), bLerp), true, false, FLinearColor::Red, 5.f);
	return InterpolatedFrame;
}

FServerSideRewindResult_Shotgun ULagCompensationComponent::ServerSideRewind_Shotgun(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> RewindFramePackages;		// ���ڴ洢��Ҫ����֡����

	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr) continue;

		const FFramePackage RewindFramePackage = GetFrameToCheck(HitCharacter, HitTime);	// ��ȡ��Ҫ����֡����
		RewindFramePackages.Add(RewindFramePackage);
	}
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerSideRewind_Shotgun: %d"), RewindFramePackages.Num()), true, false, FLinearColor::Red, 5.f);
	const FServerSideRewindResult_Shotgun RewindResult = CheckHit_Shotgun(RewindFramePackages, TraceStart, HitLocations);	// �������

	return RewindResult;
}

FServerSideRewindResult_Shotgun ULagCompensationComponent::CheckHit_Shotgun(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	//FServerSideRewindResult_Shotgun Result{};

	//if (FramePackages.Num() != HitLocations.Num()) return Result;

	//for (int32 i = 0; i < FramePackages.Num(); i++)
	//{
	//	const FFramePackage& FramePackage = FramePackages[i];
	//	ABlasterCharacter* HitCharacter = FramePackage.HitCharacter;
	//	for (const auto HitLocation : HitLocations)
	//	{
	//		const FServerSideRewindResult SingleResult = CheckHit(FramePackage, HitCharacter, TraceStart, HitLocation);	// �������
	//		Result.HeadShots.Add(FramePackage.HitCharacter, SingleResult.bHeadShot ? Result.HeadShots[FramePackage.HitCharacter] + 1 : Result.HeadShots[FramePackage.HitCharacter]);	// ���±�ͷ��Ϣ
	//		Result.BodyShots.Add(FramePackage.HitCharacter, SingleResult.bHitConfirmed && !SingleResult.bHeadShot ? Result.BodyShots[FramePackage.HitCharacter] + 1 : Result.BodyShots[FramePackage.HitCharacter]);	// ����������Ϣ
	//	}
	//}

	//return Result;

	FServerSideRewindResult_Shotgun ShotgunResult{};

	for (auto& Frame : FramePackages)
	{
		if (Frame.HitCharacter == nullptr)
		{
			// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("CheckHit_Shotgun Frame.HitCharacter == nullptr")), true, false, FLinearColor::Red, 5.f);
			return ShotgunResult;
		}
	}
	
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.HitCharacter = Frame.HitCharacter;
		CacheBoxPosition(Frame.HitCharacter, CurrentFrame);
		MoveBoxes(Frame.HitCharacter, Frame);
		EnableCharacterMeshCollision(Frame.HitCharacter, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		// Enable collision for the head first
		UBoxComponent* HeadBox = Frame.HitCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// ������ײ��Ӧ��ֻ�Կɼ���ͨ��������Ӧ
	}

	UWorld* World = GetWorld();
	// check for head shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor()))
			{
				if (UBoxComponent* HitBox = Cast<UBoxComponent>(ConfirmHitResult.Component.Get()))
				{
					DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation().Quaternion(), FColor::Red, false, 5.0f);
				}

				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// enable collision for all boxes, then disable for head box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.HitCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			FHitResult ConfirmHitResult;
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor()))
			{
				if (UBoxComponent* HitBox = Cast<UBoxComponent>(ConfirmHitResult.Component.Get()))
				{
					DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation().Quaternion(), FColor::Blue, false, 5.0f);
				}

				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.HitCharacter, Frame);
		EnableCharacterMeshCollision(Frame.HitCharacter, ECollisionEnabled::QueryAndPhysics);
	}

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("CheckHit_Shotgun: %d"), ShotgunResult.HeadShots.Num()), true, false, FLinearColor::Red, 5.f);
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("CheckHit_Shotgun: %d"), ShotgunResult.BodyShots.Num()), true, false, FLinearColor::Red, 5.f);
	return ShotgunResult;

}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
                                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	if (HitCharacter == nullptr) return;

	FServerSideRewindResult Result = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);	// �������˵���
	if (Result.bHitConfirmed && DamageCauser && Character && Character->Controller)
	{
		// �������ȷ�ϣ��ʹ����˺�
		UGameplayStatics::ApplyDamage(
			HitCharacter,	// �ܻ���ɫ
			DamageCauser->GetDamage(), // �˺�ֵ
			Character->Controller,	// �˺���Դ
			DamageCauser,	// �˺���Դ
			UDamageType::StaticClass()	// �˺�����
			);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Shotgun_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerScoreRequest_Shotgun_Implementation: %d"), HitCharacters.Num()), true, false, FLinearColor::Red, 5.f);
	const FServerSideRewindResult_Shotgun Result = ServerSideRewind_Shotgun(HitCharacters, TraceStart, HitLocations, HitTime);	// �������˵���
	for (const auto& HitCharacter : HitCharacters)
	{
		//if (DamageCauser)
		//{
		//	if (Result.HeadShots.Contains(HitCharacter))
		//	{
		//		HeadDamage = Result.HeadShots[HitCharacter] * DamageCauser->GetDamage();	// ���㱬ͷ�˺�
		//	}
		//	if (Result.BodyShots.Contains(HitCharacter))
		//	{
		//		BodyDamage = Result.BodyShots[HitCharacter] * DamageCauser->GetDamage();	// ���������˺�
		//	}
		//}

		if (Character && Character->GetEquippedWeapon() && Character->Controller)
		{
			float BodyDamage = 0.f;
			float HeadDamage = 0.f;

			if (Result.HeadShots.Contains(HitCharacter))
			{
				HeadDamage = Result.HeadShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();	// ���㱬ͷ�˺�
				// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Head Damage: %f"), HeadDamage), true, false, FLinearColor::Red, 5.f);
			}
			if (Result.BodyShots.Contains(HitCharacter))
			{
				BodyDamage = Result.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();	// ���������˺�
				// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Body Damage: %f"), BodyDamage), true, false, FLinearColor::Red, 5.f);
			}

			UGameplayStatics::ApplyDamage(
				HitCharacter,	// �ܻ���ɫ
				HeadDamage + BodyDamage, // �˺�ֵ
				Character->Controller,	// �˺���Դ
				Character->GetEquippedWeapon(),	// �˺���Դ
				UDamageType::StaticClass()	// �˺�����
			);
		}

	}
}

