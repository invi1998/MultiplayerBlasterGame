// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Pickups/HealthPickup.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();

	friend class ABlasterCharacter;		// ����ABlasterCharacter����˽�г�Ա

	void AddHealth(float HealthAmount, float HealingTime);	// ����Ѫ�� ����Ѫ��������ʱ��

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);		// ������������, ����DeltaTime, ���ڼ�����������

private:
	UPROPERTY()
	class ABlasterCharacter* Character;		// ָ��ABlasterCharacter��ָ��

	bool bHealing = false;		// �Ƿ���������

	float HealingRate = 0.0f;	// ��������

	float AmountToHeal = 0.0f;	// ������

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
