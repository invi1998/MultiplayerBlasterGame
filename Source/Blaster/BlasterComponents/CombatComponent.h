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
	void SwapWeapon();
	void Reload();

	bool IsValidSwapWeapon() const;

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwapping();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 Amount); // ����ҩ, �����������ͺ�����

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void Fire();

	void FireProjectileWeapon();

	void FireHitScanWeapon();

	void FireShotgun();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	// ����server RPC
	// ������������Ϊ�ӿͻ��˵��ã�Ȼ���ٷ���˽���ִ��
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TracerHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MuticastFire(const FVector_NetQuantize& TracerHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);		// ������������ǹ����

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);		// �ಥ����ǹ����

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	bool bLocalReloading = false;

	void DropEquippedWeapon();		// ������ǰװ��������
	void AttachActorToRightHand(AActor* ActorToAttach);		// ��ActorToAttach���ӵ�����
	void AttachActorToLeftHand(AActor* ActorToAttach);		// ��ActorToAttach���ӵ�����
	void AttachFlagToLeftHand(AWeapon* Flag);				// ��Flag���ӵ�����
	void AttachActorToBack(AActor* ActorToAttach);			// ��ActorToAttach���ӵ�����
	void UpdateCarriedAmmo();		// ����Я���ĵ�ҩ
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);	// ����װ������������	
	void ReloadEmptyWeapon();		// ����װ�������
	void ShowAttachedGrenade(bool bShowGrenade);	// ��ʾ���ӵ���

	void EquipPrimaryWeapon(class AWeapon* WeaponToEquip);		// װ��������
	void EquipSecondaryWeapon(class AWeapon* WeaponToEquip);	// װ��������

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;		// ��ǰװ��������

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	class AWeapon* SecondaryWeapon;			// ������

	// ��׼ ���ֵ�Ƿ������˵�ֵ���ͻ��˵�ֵ���ɷ�������ͬ��������
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimingButtonPressed = false;	// ��׼��ť�Ƿ���

	UFUNCTION()
	void OnRep_Aiming();

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

	TMap<EWeaponType, int32> CarriedAmmoMap;	// Я���ĵ�ҩ��������ͬ���͵�����Я���ĵ�ҩ������

	UPROPERTY(EditAnywhere)
	int32 MaxARAmmo = 300;	// ͻ����ǹ���ҩ����

	UPROPERTY(EditAnywhere)
	int32 MaxRocketAmmo = 10;	// ���Ͳ���ҩ����

	UPROPERTY(EditAnywhere)
	int32 MaxPistolAmmo = 100;	// ��ǹ���ҩ����

	UPROPERTY(EditAnywhere)
	int32 MaxSMGAmmo = 200;	// ���ǹ���ҩ����

	UPROPERTY(EditAnywhere)
	int32 MaxShotgunAmmo = 100;	// ����ǹ���ҩ����

	UPROPERTY(EditAnywhere)
	int32 MaxSniperAmmo = 50;	// �ѻ���ǹ���ҩ����

	UPROPERTY(EditAnywhere)
	int32 MaxGrenadeLauncherAmmo = 10;	// �񵯷��������ҩ����

	TMap<EWeaponType, int32> MaxCarriedAmmoMap;	// Я���ĵ�ҩ��������ͬ���͵���������Я�������ҩ������

	// ͻ����ǹ�ӵ���
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	// ���Ͳ�ڵ���
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	// ��ǹ�ӵ���
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	// ���ǹ�ӵ���
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	// ����ǹ�ӵ���
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	// �ѻ���ǹ�ӵ���
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	// �񵯷���������
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;


	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValue();

	void UpdateShotgunAmmoValue();

	UPROPERTY(ReplicatedUsing = OnRep_GrenadeCount)
	int32 GrenadeCount = 4;		// ������

	UFUNCTION()
	void OnRep_GrenadeCount();	// ������֪ͨ����

	UPROPERTY(EditAnywhere)
	int32 MaxGrenadeCount = 4;	// ���������

	void UpdateGrenadeCount();

	UPROPERTY(ReplicatedUsing = OnRep_HoldingFlag)
	bool bHoldingFlag = false;	// �Ƿ��������

	UFUNCTION()
	void OnRep_HoldingFlag();

public:
	FORCEINLINE int32 GetGrenadeCount() const { return GrenadeCount; }

};
