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

	SaveFramePackage();	// 保存帧数据

}

FFramePackage ULagCompensationComponent::InterpolateFrame(const FFramePackage& OlderFrame,
	const FFramePackage& NewerFrame, float HitTime) const
{
	FFramePackage InterpolatedFrame{};
	const float Alpha = FMath::Clamp((HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time), 0.f, 1.f);	// 插值公式：(HitTime - OlderFrame.Time) / (NewerFrame.Time - OlderFrame.Time)
	for(auto& BoxPair : OlderFrame.HitBoxInfo)
	{
		const FBoxInformation& OlderBoxInfo = BoxPair.Value;
		const FBoxInformation& NewerBoxInfo = NewerFrame.HitBoxInfo[BoxPair.Key];

		FBoxInformation InterpolatedBoxInfo;
		InterpolatedBoxInfo.Location = FMath::Lerp(OlderBoxInfo.Location, NewerBoxInfo.Location, Alpha);
		InterpolatedBoxInfo.Extent = NewerBoxInfo.Extent;		// 这里千万不能差值，否则会导致命中框大小变化（因为命中框大小是固定的）
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
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// 设置碰撞响应

	FHitResult HitResult;	// 射线检测
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;	// 射线终点
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);	// 射线检测

		if (HitResult.bBlockingHit)		// 命中
		{
			if (HitResult.Component.IsValid())
			{
				if (UBoxComponent* HitBox = Cast<UBoxComponent>(HitResult.Component.Get()))
				{
					DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation().Quaternion(), FColor::Red, false, 5.0f);
				}
			}

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
					BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// 设置碰撞响应
				}
			}

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);	// 射线检测
			if (HitResult.bBlockingHit)		// 命中
			{
				if (HitResult.Component.IsValid())
				{
					if (UBoxComponent* HitBox = Cast<UBoxComponent>(HitResult.Component.Get()))
					{
						DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation().Quaternion(), FColor::Blue, false, 5.0f);
					}
				}

				ResetHitBoxes(HitCharacter, CurrentFrame);	// 重置命中框
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// 启用角色的碰撞
				Result.bHitConfirmed = true;	// 命中确认
				Result.bHeadShot = false;	// 判断是否是爆头
				return Result;
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);	// 重置命中框
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);	// 启用角色的碰撞

	return Result;
}

void ULagCompensationComponent::CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 遍历命中框
		if (HitBoxPair.Value != nullptr)	// 如果命中框不为空
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();	// 获取命中框的位置
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();	// 获取命中框的旋转
			BoxInfo.Extent = HitBoxPair.Value->GetScaledBoxExtent();		// 获取命中框的大小
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);

			DrawDebugBox(GetWorld(), BoxInfo.Location, BoxInfo.Extent, BoxInfo.Rotation.Quaternion(), FColor::Orange, false, 5.0f);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)	// 遍历命中框
	{
		if (HitBoxPair.Value != nullptr)	// 如果命中框不为空
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfo[HitBoxPair.Key].Location);	// 设置命中框的位置
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfo[HitBoxPair.Key].Rotation);	// 设置命中框的旋转
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfo[HitBoxPair.Key].Extent);			// 设置命中框的大小

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
		return FFramePackage();	// 如果最老的帧数据的时间大于命中时间，就直接返回空
	}

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

		OlderNode = OlderNode->GetNextNode();

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

	InterpolatedFrame.HitCharacter = HitCharacter;

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("GetFrameToCheck bLerp: %d"), bLerp), true, false, FLinearColor::Red, 5.f);
	return InterpolatedFrame;
}

FServerSideRewindResult_Shotgun ULagCompensationComponent::ServerSideRewind_Shotgun(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> RewindFramePackages;		// 用于存储需要检查的帧数据

	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr) continue;

		const FFramePackage RewindFramePackage = GetFrameToCheck(HitCharacter, HitTime);	// 获取需要检查的帧数据
		RewindFramePackages.Add(RewindFramePackage);
	}
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerSideRewind_Shotgun: %d"), RewindFramePackages.Num()), true, false, FLinearColor::Red, 5.f);
	const FServerSideRewindResult_Shotgun RewindResult = CheckHit_Shotgun(RewindFramePackages, TraceStart, HitLocations);	// 检查命中

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
	//		const FServerSideRewindResult SingleResult = CheckHit(FramePackage, HitCharacter, TraceStart, HitLocation);	// 检查命中
	//		Result.HeadShots.Add(FramePackage.HitCharacter, SingleResult.bHeadShot ? Result.HeadShots[FramePackage.HitCharacter] + 1 : Result.HeadShots[FramePackage.HitCharacter]);	// 更新爆头信息
	//		Result.BodyShots.Add(FramePackage.HitCharacter, SingleResult.bHitConfirmed && !SingleResult.bHeadShot ? Result.BodyShots[FramePackage.HitCharacter] + 1 : Result.BodyShots[FramePackage.HitCharacter]);	// 更新身体信息
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
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// 设置碰撞响应，只对可见性通道进行响应
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

	FServerSideRewindResult Result = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);	// 服务器端倒带
	if (Result.bHitConfirmed && DamageCauser && Character && Character->Controller)
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

void ULagCompensationComponent::ServerScoreRequest_Shotgun_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("ServerScoreRequest_Shotgun_Implementation: %d"), HitCharacters.Num()), true, false, FLinearColor::Red, 5.f);
	const FServerSideRewindResult_Shotgun Result = ServerSideRewind_Shotgun(HitCharacters, TraceStart, HitLocations, HitTime);	// 服务器端倒带
	for (const auto& HitCharacter : HitCharacters)
	{
		//if (DamageCauser)
		//{
		//	if (Result.HeadShots.Contains(HitCharacter))
		//	{
		//		HeadDamage = Result.HeadShots[HitCharacter] * DamageCauser->GetDamage();	// 计算爆头伤害
		//	}
		//	if (Result.BodyShots.Contains(HitCharacter))
		//	{
		//		BodyDamage = Result.BodyShots[HitCharacter] * DamageCauser->GetDamage();	// 计算身体伤害
		//	}
		//}

		if (Character && Character->GetEquippedWeapon() && Character->Controller)
		{
			float BodyDamage = 0.f;
			float HeadDamage = 0.f;

			if (Result.HeadShots.Contains(HitCharacter))
			{
				HeadDamage = Result.HeadShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();	// 计算爆头伤害
				// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Head Damage: %f"), HeadDamage), true, false, FLinearColor::Red, 5.f);
			}
			if (Result.BodyShots.Contains(HitCharacter))
			{
				BodyDamage = Result.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();	// 计算身体伤害
				// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Body Damage: %f"), BodyDamage), true, false, FLinearColor::Red, 5.f);
			}

			UGameplayStatics::ApplyDamage(
				HitCharacter,	// 受击角色
				HeadDamage + BodyDamage, // 伤害值
				Character->Controller,	// 伤害来源
				Character->GetEquippedWeapon(),	// 伤害来源
				UDamageType::StaticClass()	// 伤害类型
			);
		}

	}
}

