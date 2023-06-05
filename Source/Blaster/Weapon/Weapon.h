// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

// 武器状态枚举，其枚举常量是无符号八位整数，同时这个也是一个蓝图类型
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),		// 武器在世界上放置时的状态
	EWS_Equipped UMETA(DisplayName = "Equipped"),			// 武器被装备时的状态
	EWS_Dropped UMETA(DisplayName = "Dropped"),				// 武器被丢弃时的状态
	// 这些就是我们可以用于控制打开物理，打开碰撞，让武器反弹的地方

	EWS_MAX UMETA(DisplayName = "DefaultMax")				// 默认最大常量
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	/*
	 * 十字准心纹理
	 * 这里有几个变量，因为我想要控制左右上下以及中心的纹理
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsBottom;

	// 瞄准时的武器缩放 FOV
	UPROPERTY(EditAnywhere)
		float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
		float ZoomInterpSpeed = 20.f;

	/*
	 * 自动开火
	 */

	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = 0.15f;

	// 标志是否是自动武器（自动开火的武器）
	UPROPERTY(EditAnywhere, Category = Combat)
		bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = Weapon)
	class USoundCue* EquipSound;

public:
	// Sets default values for this actor's properties
	AWeapon();
	virtual void OnRep_Owner() override;
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	void SetHUDAmmo();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	// 丢弃武器 / 死亡掉落武器
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,	// 原始组件
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,	// 原始组件
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

private:
	// 武器骨架网格
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickupWidget;

	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
		int32 Ammo;

	UFUNCTION()
		void OnRep_Ammo();

	// 扣除弹药，检测武器是否有一个有效的所有者
	void SpendRound();

	UPROPERTY(EditAnywhere)
		int32 MagCapacity;

	UPROPERTY()
		class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
		class ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;

public:
	// Called every frame

	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }

	bool IsEmpty();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
