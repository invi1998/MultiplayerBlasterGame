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

	UPROPERTY()
	ABlasterCharacter* HitCharacter;	// 命中角色
};

// 普通武器的服务端倒带结果
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;	// 命中确认

	UPROPERTY()
	bool bHeadShot;		// 是否是爆头

};

// 霰弹枪的服务端倒带结果
USTRUCT(BlueprintType)
struct FServerSideRewindResult_Shotgun
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;		// 爆头次数

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;		// 身体命中次数
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

	FServerSideRewindResult ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,  const FVector_NetQuantize& HitLocation, float HitTime);	// 服务器端倒带，传入命中角色，射线起始位置，命中位置，命中时间

	FServerSideRewindResult_Shotgun ServerSideRewind_Shotgun(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);	// 服务器端倒带，传入命中角色，射线起始位置，命中位置，命中时间

	FServerSideRewindResult ServerSideRewind_Projectile(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);	// 服务器端倒带，传入命中角色，投射起始位置，投射初始速度，命中时间

	/*
	 * 射线类型的服务端倒带（SSR for Hit Scan）
	 */
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser);	// 服务器端请求得分，传入命中角色，射线起始位置，命中位置，命中时间, 伤害来源

	/*
	 * 霰弹枪类型的服务端倒带（SSR for Shotgun）
	 */
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest_Shotgun(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime, class AWeapon* DamageCauser);	// 服务器端请求得分，传入命中角色，射线起始位置，命中位置，命中时间, 伤害来源

	/*
	 * 投射类型的服务端倒带（SSR for Projectile）
	 */
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest_Projectile(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime, class AWeapon* DamageCauser);	// 服务器端请求得分，传入命中角色，投射起始位置，投射初始速度，命中时间, 伤害来源


protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);		// 保存帧数据

	FFramePackage InterpolateFrame(const FFramePackage& OlderFrame, const FFramePackage& NewerFrame, float HitTime) const;	// 插值帧数据

	void CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);	// 缓存命中框位置，传入命中角色，输出帧数据

	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage);	// 移动命中框，传入命中角色，帧数据

	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage);	// 重置命中框，传入命中角色, 帧数据

	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type Collision);	// 启用角色碰撞，传入命中角色，是否启用

	void SaveFramePackage();	// 保存帧数据到历史记录

	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);	// 获取用于检查的帧数据，传入命中角色，命中时间

	/*
	 * 射线类型的命中检查
	 */
	FServerSideRewindResult CheckHit(const FFramePackage& FramePackage, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);	// 检查命中，传入帧数据，命中角色，射线起始位置，命中位置

	/*
	 * 霰弹枪
	 */
	FServerSideRewindResult_Shotgun CheckHit_Shotgun(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);	// 检查命中，传入帧数据，命中角色，射线起始位置，命中位置

	/*
	 * 投射类型的命中检查
	 */
	FServerSideRewindResult CheckHit_Projectile(const FFramePackage& FramePackage, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity);	// 检查命中，传入帧数据，命中角色，投射起始位置，投射初始速度


private:
	UPROPERTY()
	ABlasterCharacter* Character;	// 拥有该组件的角色

	UPROPERTY()
	class ABlasterPlayerController* Controller;	// 玩家控制器

	TDoubleLinkedList<FFramePackage> FrameHistory;	// 历史帧数据链表

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 5.0f;	// 最大记录时间

};
