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

	void PickupAmmo(EWeaponType WeaponType, int32 Amount); // 捡起弹药, 传入武器类型和数量

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

	// 创建server RPC
	// 这个函数被设计为从客户端调用，然后再服务端进行执行
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TracerHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MuticastFire(const FVector_NetQuantize& TracerHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);		// 服务器端霰弹枪开火

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);		// 多播霰弹枪开火

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

	void DropEquippedWeapon();		// 丢弃当前装备的武器
	void AttachActorToRightHand(AActor* ActorToAttach);		// 将ActorToAttach附加到右手
	void AttachActorToLeftHand(AActor* ActorToAttach);		// 将ActorToAttach附加到左手
	void AttachFlagToLeftHand(AWeapon* Flag);				// 将Flag附加到左手
	void AttachActorToBack(AActor* ActorToAttach);			// 将ActorToAttach附加到背部
	void UpdateCarriedAmmo();		// 更新携带的弹药
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);	// 播放装备武器的声音	
	void ReloadEmptyWeapon();		// 重新装填空武器
	void ShowAttachedGrenade(bool bShowGrenade);	// 显示附加的榴弹

	void EquipPrimaryWeapon(class AWeapon* WeaponToEquip);		// 装备主武器
	void EquipSecondaryWeapon(class AWeapon* WeaponToEquip);	// 装备副武器

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;		// 当前装备的武器

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	class AWeapon* SecondaryWeapon;			// 副武器

	// 瞄准 这个值是服务器端的值，客户端的值是由服务器端同步过来的
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimingButtonPressed = false;	// 瞄准按钮是否按下

	UFUNCTION()
	void OnRep_Aiming();

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

	TMap<EWeaponType, int32> CarriedAmmoMap;	// 携带的弹药数量（不同类型的武器携带的弹药数量）

	UPROPERTY(EditAnywhere)
	int32 MaxARAmmo = 300;	// 突击步枪最大弹药数量

	UPROPERTY(EditAnywhere)
	int32 MaxRocketAmmo = 10;	// 火箭筒最大弹药数量

	UPROPERTY(EditAnywhere)
	int32 MaxPistolAmmo = 100;	// 手枪最大弹药数量

	UPROPERTY(EditAnywhere)
	int32 MaxSMGAmmo = 200;	// 冲锋枪最大弹药数量

	UPROPERTY(EditAnywhere)
	int32 MaxShotgunAmmo = 100;	// 霰弹枪最大弹药数量

	UPROPERTY(EditAnywhere)
	int32 MaxSniperAmmo = 50;	// 狙击步枪最大弹药数量

	UPROPERTY(EditAnywhere)
	int32 MaxGrenadeLauncherAmmo = 10;	// 榴弹发射器最大弹药数量

	TMap<EWeaponType, int32> MaxCarriedAmmoMap;	// 携带的弹药数量（不同类型的武器允许携带的最大弹药数量）

	// 突击步枪子弹数
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	// 火箭筒炮弹数
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	// 手枪子弹数
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	// 冲锋枪子弹数
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	// 霰弹枪子弹数
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	// 狙击步枪子弹数
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	// 榴弹发射器榴弹数
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
	int32 GrenadeCount = 4;		// 榴弹数量

	UFUNCTION()
	void OnRep_GrenadeCount();	// 榴弹数量通知函数

	UPROPERTY(EditAnywhere)
	int32 MaxGrenadeCount = 4;	// 最大榴弹数量

	void UpdateGrenadeCount();

	UPROPERTY(ReplicatedUsing = OnRep_HoldingFlag)
	bool bHoldingFlag = false;	// 是否持有旗帜

	UFUNCTION()
	void OnRep_HoldingFlag();

public:
	FORCEINLINE int32 GetGrenadeCount() const { return GrenadeCount; }

};
