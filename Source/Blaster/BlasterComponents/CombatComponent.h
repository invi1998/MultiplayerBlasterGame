// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	// ����server RPC
	// ������������Ϊ�ӿͻ��˵��ã�Ȼ���ٷ���˽���ִ��
	UFUNCTION(Server, Reliable)
		void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
		void MuticastFire();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	class ABlasterCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		class AWeapon* EquippedWeapon;

	// ��׼
	UPROPERTY(Replicated)
		bool bAiming;

	// �н�����
	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;

	// ��׼�о�����
	UPROPERTY(EditAnywhere)
		float AimWalkSpeed;

	// ����ť�Ƿ���
	bool bFireButtonPressed;

public:
	// Called every frame
};
