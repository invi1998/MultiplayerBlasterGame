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

	// 创建server RPC
	// 这个函数被设计为从客户端调用，然后再服务端进行执行
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

	// 瞄准
	UPROPERTY(Replicated)
		bool bAiming;

	// 行进速率
	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;

	// 瞄准行径速率
	UPROPERTY(EditAnywhere)
		float AimWalkSpeed;

	// 开火按钮是否按下
	bool bFireButtonPressed;

	// 保存准心瞄准的目标
	//FVector HitTarget;

	/*
	 * HUD 和 十字准心
	 * 这个控制因子作用就是基于角色移动速度来控制我们的十字准心的伸展速率
	 * 基于角色是否浮空
	 * 基于角色是否瞄准
	 * 基于角色是否在射击
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

	 // 不瞄准时的视野。设置为相机的基本视野或FMV并开始播放。
	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	 * 自动开火
	 */

	FTimerHandle FireTimer;

	// 标志是否能开火
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// 当前武器携带的弹药数量
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
