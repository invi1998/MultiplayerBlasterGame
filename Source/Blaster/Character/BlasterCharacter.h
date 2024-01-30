// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	// 播放开火蒙太奇动画
	void PlayFireMontage(bool bAiming);

	// 播放开火蒙太奇动画
	void PlayReloadMontage();

	// 播放死亡飞升蒙太奇动画
	void PlayElimMontage();

	// 播放人物受击的蒙太奇动画
	void PlayHitReactMontage();

	// 播放投掷手榴弹的蒙太奇动画
	void PlayThrowGrenadeMontage();

	//// 人物受击rpc函数（多播）
	//UFUNCTION(NetMulticast, Unreliable)
	//	void MuticastHit();

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
		bool bDisableGamePlay = false;

	// 显示狙击步枪的瞄准镜	ui
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();	// 更新HUD血量

	void UpdateHUDShield();	// 更新HUD护盾

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 各种操作映射函数

	void MoveForward(float Value);

	void MoveRight(float Value);

	void Turn(float Value);

	void LookUp(float Value);

	// 按E装备武器
	void EquipButtonPressed();

	// 按ctrl蹲伏和起身
	void CrouchButtonPressed();

	void ReloadButtonPressed();	// 按R换弹

	// 鼠标右键瞄准
	void AimButtonPressed();

	// 松开右键瞄准
	void AimButtonReleased();
	void CalculateAO_Pitch();	// 计算目标偏移量 pitch 轴

	// 设置目标偏移量
	void AimOffset(float DeltaTime);

	// 模拟代理
	void SimProxiesTurn();

	// 原本Actor的jump逻辑是，在蹲下的时候是不可以跳跃的，为了实现蹲下的时候，按空格可以取消蹲伏，这里需要重写jump
	virtual void Jump() override;

	// 点击开火（鼠标左键按下）
	void FireButtonPressed();
	// 点击开火（鼠标左键抬起）
	void FireButtonReleased();
	

	// 手榴弹投掷按钮按下（T）
	void GrenadeButtonPressed();

	// 角色受到伤害处理函数
	UFUNCTION()
		void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// 初始化HUD
	void PollInit();

	// 原地旋转
	void RotatePlace(float DeltaTime);

private:
	// 添加弹簧臂和摄像机

	// 属性宏，让其在任何地方可见，想将相机，吊杆归为一类，叫做相机
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;	// 吊杆

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;	// 摄像机

	// 既然我们要从蓝图访问我们的部件，至少需要设置蓝图只读
	// 如果将私有变量设置蓝图只读，那么就需要允许私人访问，这意味着将这个变量暴露给蓝图
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;	// 顶部小部件

	// 武器类的前向声明
	// 我们想要复制这个变量，这意味着当它在服务器上发生变化时，我们想要同步在客户端上也进行更改，我们可以复制指向武器的指针
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);	// 当OverlappingWeapon发生变化时，调用该函数

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
		class UCombatComponent* Combat;		// 战斗组件

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;		// buff组件

	UFUNCTION(Server, Reliable)
		void ServerEquipButtonPressed();

	float AO_Yaw;	// 目标偏移量 yaw 轴
	float InterpAO_Yaw;	// 插值目标偏移量 yaw 轴

	float AO_Pitch;	// 目标偏移量 pitch 轴

	// 起始旋转信息
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;		// 旋转状态 0：不旋转 1：旋转 2：旋转完成

	void TurnInPlace(float DeltaTime);		// 旋转

	// 武器开火的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;

	// 换弹动作蒙太奇动画
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ReloadMontage;

	// 人物受击动作的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* HitReactMontage;

	// 角色死亡飞升的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ElimMontage;

	// 投掷手榴弹的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ThrowGrenadeMontage;

	// 如果角色靠太近就隐藏摄像头
	void HidCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
		float CameraThreshold = 200.f;

	// 是否旋转根骨骼
	bool bRotateRootBone;

	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/*
	 * 玩家血量
	 */

	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;

	// 玩家血量，可被血量通知函数复制
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
		float Health = 100.f;

	// 将玩家血量更新给客户端
	UFUNCTION()
		void OnRep_Health(float LastHealth);

	/*
	 * 玩家护盾
	 */
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY()
		class ABlasterPlayerController* BlasterPlayerController;

	// 玩家是否被击败（死亡）
	bool bElimmed = false;

	// 玩家死亡复活倒计时时钟
	FTimerHandle ElimTimer;

	// 当复活倒计时结束时的处理逻辑
	void ElimTimerFinished();

	// 复活时间
	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.f;

	/*
	 * 角色溶解效果
	 */
	UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	UPROPERTY(EditAnywhere)
		UCurveFloat* DissolveCurve;

	// 在游戏运行时更改的动态实例，这个实例在这里将会是材质实例
	// 动态溶解材质，用于存储动态创建的材质实例
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// 将分配给角色蓝图的材质实例
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;

	/*
	 * Elim Bot
	 */

	UPROPERTY(EditAnywhere)
		UParticleSystem* ElimBotEffect;

	UPROPERTY(EditAnywhere)
		UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
		class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	/**
	 * 手榴弹
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	// 是否装备武器
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGamePlay() const { return bDisableGamePlay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
};
