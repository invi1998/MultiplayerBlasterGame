// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * ����
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

	virtual void Dropped() override;	// ��д��������
	void ResetFlag();	// ��������

protected:
	virtual void BeginPlay() override;	// ��д��ʼ����
	virtual void OnEquipped() override;	// ��дװ��ʱ�ķ���
	virtual void OnDropped() override;	// ��д����ʱ�ķ���

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* FlagMesh;	// ����ģ��

	FTransform InitialTransform;	// ��ʼλ�ã���ת������

public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }	// ��ȡ��ʼλ��
};
