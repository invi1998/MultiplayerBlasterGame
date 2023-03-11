// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
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
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_EquippedWeapon();

	void Fire();

	void FireButtonPressed(bool bPressed);

	// ����server RPC
	// ������������Ϊ�ӿͻ��˵��ã�Ȼ���ٷ���˽���ִ��
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TracerHitTarget);

	UFUNCTION(NetMulticast, Reliable)
		void MuticastFire(const FVector_NetQuantize& TracerHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
		void ServerReload();


	void HandleReload();

	int32 AmountToReload();

private:
	UPROPERTY()
		class ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;
	UPROPERTY()
		class ABlasterHUD* HUD;

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

	// ����׼����׼��Ŀ��
	//FVector HitTarget;

	/*
	 * HUD �� ʮ��׼��
	 * ��������������þ��ǻ��ڽ�ɫ�ƶ��ٶ����������ǵ�ʮ��׼�ĵ���չ����
	 * ���ڽ�ɫ�Ƿ񸡿�
	 * ���ڽ�ɫ�Ƿ���׼
	 * ���ڽ�ɫ�Ƿ������
	 */
	float CrosshairVelocityFacor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	/*
	 * Aiming and FOV
	 */

	 // ����׼ʱ����Ұ������Ϊ����Ļ�����Ұ��FMV����ʼ���š�
	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	 * �Զ�����
	 */

	FTimerHandle FireTimer;

	// ��־�Ƿ��ܿ���
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// ��ǰ����Я���ĵ�ҩ����
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
		int32 CarriedAmmo;

	UFUNCTION()
		void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
		int32 StartingARAmmo = 30;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValue();

public:
	// Called every frame
};
