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

	// 获取角色速度矢量
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Spead = Velocity.Size();

	// 通过获取角色是否处于掉落状态来 赋值 bIsInAir
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	// 判断 获取当前加速度 是否大于0来判断是否处于加速状态
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	// 判断当前角色是否装备了武器，将状态赋值给 bWeaponEquipped
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();

	bIsCrouched = BlasterCharacter->bIsCrouched;
}