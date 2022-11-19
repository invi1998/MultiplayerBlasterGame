// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
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

	// 设置目标偏移量
	void AimOffset(float DeltaTime);

	// 原本Actor的jump逻辑是，在蹲下的时候是不可以跳跃的，为了实现蹲下的时候，按空格可以取消蹲伏，这里需要重写jump
	virtual void Jump() override;

	// 点击开火（鼠标左键按下）
	void FireButtonPressed();
	// 点击开火（鼠标左键抬起）
	void FireButtonReleased();

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

	// 定义一个开火武器的动画蒙太奇指针
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	// 是否装备武器
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
};
