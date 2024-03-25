// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;	// 位置

	UPROPERTY()
	FVector Extent;		// 大小

	UPROPERTY()
	FRotator Rotation;	// 旋转
};

// 用于存储每一帧的数据（存储命中框信息的数据结构）
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;		// 命中时间

	TMap<FName, FBoxInformation> HitBoxInfo;	// 命中框信息
};

// 处理玩家延迟的组件（服务器倒带）

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();

	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);	// 显示帧数据

	void ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,  const FVector_NetQuantize& HitLocation, float HitTime);	// 服务器端倒带，传入命中角色，射线起始位置，命中位置，命中时间

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);		// 保存帧数据

	FFramePackage InterpolateFrame(const FFramePackage& OlderFrame, const FFramePackage& NewerFrame, float HitTime);	// 插值帧数据

private:
	UPROPERTY()
	ABlasterCharacter* Character;	// 拥有该组件的角色

	UPROPERTY()
	class ABlasterPlayerController* Controller;	// 玩家控制器

	TDoubleLinkedList<FFramePackage> FrameHistory;	// 历史帧数据链表

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 5.0f;	// 最大记录时间

};
