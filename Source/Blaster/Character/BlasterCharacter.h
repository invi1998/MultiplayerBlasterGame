// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
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

	// 人物受击rpc函数（多播）
	UFUNCTION(NetMulticast, Unreliable)
		void MuticastHit();

	virtual void OnRep_ReplicatedMovement() override;

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

	// 鼠标右键瞄准
	void AimButtonPressed();

	// 松开右键瞄准
	void AimButtonReleased();
	void CalculateAO_Pitch();

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

	// 播放人物受击的蒙太奇动画
	void PlayHitReactMontage();

private:
	// 添加弹簧臂和摄像机

	// 属性宏，让其在任何地方可见，想将相机，吊杆归为一类，叫做相机
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	// 既然我们要从蓝图访问我们的部件，至少需要设置蓝图只读
	// 如果将私有变量设置蓝图只读，那么就需要允许私人访问，这意味着将这个变量暴露给蓝图
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;

	// 武器类的前向声明
	// 我们想要复制这个变量，这意味着当它在服务器上发生变化时，我们想要同步在客户端上也进行更改，我们可以复制指向武器的指针
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
		void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;

	float AO_Pitch;

	// 起始旋转信息
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	// 武器开火的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;

	// 人物受击动作的动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* HitReactMontage;

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
		void OnRep_Health();

	class ABlasterPlayerController* BlasterPlayerController;

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
};
