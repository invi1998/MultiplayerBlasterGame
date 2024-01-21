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
	bHealing = true;		// ������������Ϊtrue
	HealingRate = HealthAmount / HealingTime;		// ������������
	AmountToHeal = HealthAmount;		// ������������һ���������ﵽ��HealthAmount���ͻ�ֹͣ����
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;		// ���û�����ƻ���CharacterΪ�գ�����Character�Ѿ�����̭���ͷ���

	const float HealThisFrame = HealingRate * DeltaTime;		// ���㱾֡������
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.0f, Character->GetMaxHealth()));		// ����Ѫ����ʹ��FMath::Clamp()����������Ѫ����0�����Ѫ��֮��
	Character->UpdateHUDHealth();		// ����Ѫ��UI
	AmountToHeal -= HealThisFrame;		// ��������ȥ��֡������

	if (AmountToHeal <= 0.0f || Character->GetHealth() >= Character->GetMaxHealth())		// ���������С�ڵ���0 ���� Ѫ�����ڵ������Ѫ��
	{
		bHealing = false;		// ������������Ϊfalse
		HealingRate = 0.0f;		// ������������Ϊ0
		AmountToHeal = 0.0f;		// ����������Ϊ0
	}
}


// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ÿ֡����HealRampUp()����
	HealRampUp(DeltaTime);

}

