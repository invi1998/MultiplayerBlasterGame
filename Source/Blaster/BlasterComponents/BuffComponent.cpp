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
	bHealing = true;		// ������������Ϊtrue
	HealingRate = HealthAmount / HealingTime;		// ������������
	AmountToHeal = HealthAmount;		// ������������һ���������ﵽ��HealthAmount���ͻ�ֹͣ����
}

void UBuffComponent::AddSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;		// ���CharacterΪ�գ��ͷ���

	if (SpeedBuffTimerHandle.IsValid())		// �����ʱ����Ч
	{
		Character->GetWorldTimerManager().ClearTimer(SpeedBuffTimerHandle);		// �����ʱ��
	}
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimerHandle, this, &UBuffComponent::ResetSpeed, BuffTime);		// ���ü�ʱ��

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;		// ������������ٶ�
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;		// �����������ٶ�
	}

	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed, BuffTime);		// �ಥ�ٶ�buff
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;		// ���ó�ʼ�����ٶ�
	InitialCrouchSpeed = CrouchSpeed;	// ���ó�ʼ�����ٶ�
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


void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;		// ���CharacterΪ�ջ���Character��CharacterMovementΪ�գ��ͷ���

	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;		// ������������ٶ�
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;		// �����������ٶ�

	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed, 0.0f);		// �ಥ�ٶ�buff
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;		// ������������ٶ�
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;		// �����������ٶ�
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ÿ֡����HealRampUp()����
	HealRampUp(DeltaTime);

}

