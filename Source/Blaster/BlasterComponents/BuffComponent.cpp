// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UBuffComponent::AddHealth(float HealthAmount, float HealingTime)
{
	bHealing = true;		// 设置正在治疗为true
	HealingRate = HealthAmount / HealingTime;		// 计算治疗速率
	AmountToHeal = HealthAmount;		// 计算治疗量，一旦治疗量达到了HealthAmount，就会停止治疗
}

void UBuffComponent::AddSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;		// 如果Character为空，就返回

	if (SpeedBuffTimerHandle.IsValid())		// 如果计时器有效
	{
		Character->GetWorldTimerManager().ClearTimer(SpeedBuffTimerHandle);		// 清除计时器
	}
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimerHandle, this, &UBuffComponent::ResetSpeed, BuffTime);		// 设置计时器

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;		// 设置最大行走速度
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;		// 设置最大蹲下速度
	}

	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed, BuffTime);		// 多播速度buff
}

void UBuffComponent::AddJump(float JumpZVelocity, float JumpTime)
{
	if (Character == nullptr) return;		// 如果Character为空，就返回

	if (JumpBuffTimerHandle.IsValid())
	{
		Character->GetWorldTimerManager().ClearTimer(JumpBuffTimerHandle);		// 清除计时器
	}

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimerHandle, this, &UBuffComponent::RestJump, JumpTime);		// 设置计时器

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;		// 设置跳跃速度
	}

	MulticastJumpBuff(JumpZVelocity, JumpTime);		// 多播跳跃buff
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;		// 设置初始基础速度
	InitialCrouchSpeed = CrouchSpeed;	// 设置初始蹲下速度
}

void UBuffComponent::SetInitialJumpZVelocity(float JumpZVelocity)
{
	InitialJumpZVelocity = JumpZVelocity;		// 设置初始跳跃速度
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;		// 如果没有治疗或者Character为空，或者Character已经被淘汰，就返回

	const float HealThisFrame = HealingRate * DeltaTime;		// 计算本帧治疗量
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.0f, Character->GetMaxHealth()));		// 设置血量，使用FMath::Clamp()方法来限制血量在0到最大血量之间
	Character->UpdateHUDHealth();		// 更新血量UI
	AmountToHeal -= HealThisFrame;		// 治疗量减去本帧治疗量

	if (AmountToHeal <= 0.0f || Character->GetHealth() >= Character->GetMaxHealth())		// 如果治疗量小于等于0 或者 血量大于等于最大血量
	{
		bHealing = false;		// 设置正在治疗为false
		HealingRate = 0.0f;		// 设置治疗速率为0
		AmountToHeal = 0.0f;		// 设置治疗量为0
	}
}


void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;		// 如果Character为空或者Character的CharacterMovement为空，就返回

	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;		// 设置最大行走速度
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;		// 设置最大蹲下速度

	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed, 0.0f);		// 多播速度buff
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;		// 如果Character为空或者Character的CharacterMovement为空，就返回

	Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;		// 设置最大行走速度
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;		// 设置最大蹲下速度
}

void UBuffComponent::RestJump()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;		// 如果Character为空或者Character的CharacterMovement为空，就返回

	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity;		// 设置跳跃速度

	MulticastJumpBuff(InitialJumpZVelocity, 0.0f);		// 多播跳跃buff
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpZVelocity, float JumpTime)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;		// 如果Character为空或者Character的CharacterMovement为空，就返回

	Character->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;		// 设置跳跃速度
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 每帧调用HealRampUp()方法
	HealRampUp(DeltaTime);

}

