// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"

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


// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 每帧调用HealRampUp()方法
	HealRampUp(DeltaTime);

}

