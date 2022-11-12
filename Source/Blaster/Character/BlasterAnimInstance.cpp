// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharacter == nullptr)
	{
		return;
	}

	// ��ȡ��ɫ�ٶ�ʸ��
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Spead = Velocity.Size();

	// ͨ����ȡ��ɫ�Ƿ��ڵ���״̬�� ��ֵ bIsInAir
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	// �ж� ��ȡ��ǰ���ٶ� �Ƿ����0���ж��Ƿ��ڼ���״̬
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	// �жϵ�ǰ��ɫ�Ƿ�װ������������״̬��ֵ�� bWeaponEquipped
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();

	bIsCrouched = BlasterCharacter->bIsCrouched;
}