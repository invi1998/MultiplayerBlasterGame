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
	FVector Location;	// λ��

	UPROPERTY()
	FVector Extent;		// ��С

	UPROPERTY()
	FRotator Rotation;	// ��ת
};

// ���ڴ洢ÿһ֡�����ݣ��洢���п���Ϣ�����ݽṹ��
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;		// ����ʱ��

	TMap<FName, FBoxInformation> HitBoxInfo;	// ���п���Ϣ
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;	// ����ȷ��

	UPROPERTY()
	bool bHeadShot;		// �Ƿ��Ǳ�ͷ

};

// ��������ӳٵ������������������

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();

	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);	// ��ʾ֡����

	FServerSideRewindResult ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,  const FVector_NetQuantize& HitLocation, float HitTime);	// �������˵������������н�ɫ��������ʼλ�ã�����λ�ã�����ʱ��

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser);	// ������������÷֣��������н�ɫ��������ʼλ�ã�����λ�ã�����ʱ��, �˺���Դ

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);		// ����֡����

	FFramePackage InterpolateFrame(const FFramePackage& OlderFrame, const FFramePackage& NewerFrame, float HitTime);	// ��ֵ֡����

	FServerSideRewindResult CheckHit(const FFramePackage& FramePackage, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);	// ������У�����֡���ݣ����н�ɫ��������ʼλ�ã�����λ��

	void CacheBoxPosition(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);	// �������п�λ�ã��������н�ɫ�����֡����

	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage);	// �ƶ����п򣬴������н�ɫ��֡����

	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage);	// �������п򣬴������н�ɫ, ֡����

	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type Collision);	// ���ý�ɫ��ײ���������н�ɫ���Ƿ�����

	void SaveFramePackage();	// ����֡���ݵ���ʷ��¼

	/*
	 * ����ǹ
	 */
	FFramePackage InterpolateFrameForShotgun(const FFramePackage& OlderFrame, const FFramePackage& NewerFrame, float HitTime);	// ��ֵ֡����

private:
	UPROPERTY()
	ABlasterCharacter* Character;	// ӵ�и�����Ľ�ɫ

	UPROPERTY()
	class ABlasterPlayerController* Controller;	// ��ҿ�����

	TDoubleLinkedList<FFramePackage> FrameHistory;	// ��ʷ֡��������

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 5.0f;	// ����¼ʱ��

};
