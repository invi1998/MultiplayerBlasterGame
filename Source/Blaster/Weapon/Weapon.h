// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

// ����״̬ö�٣���ö�ٳ������޷��Ű�λ������ͬʱ���Ҳ��һ����ͼ����
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),		// �����������Ϸ���ʱ��״̬
	EWS_Equipped UMETA(DisplayName = "Equipped"),			// ������װ��ʱ��״̬
	EWS_Dropped UMETA(DisplayName = "Dropped"),				// ����������ʱ��״̬
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),	// ������װ��Ϊ��Ҫ����ʱ��״̬
	// ��Щ�������ǿ������ڿ��ƴ���������ײ�������������ĵط�

	EWS_MAX UMETA(DisplayName = "DefaultMax")				// Ĭ�������
};

UENUM(BlueprintType)
enum class EFiringType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),		// ɨ��
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),			// Ͷ��
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),			// ����ǹ

	EFT_MAX UMETA(DisplayName = "DefaultMax")				// Ĭ�������
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	/*
	 * ʮ��׼������
	 * �����м�����������Ϊ����Ҫ�������������Լ����ĵ�����
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

	// ��׼ʱ���������� FOV
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/*
	 * �Զ�����
	 */

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;

	// ��־�Ƿ����Զ��������Զ������������
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = Weapon)
	class USoundCue* EquipSound;

	/*
	 * ���û��߽����Զ������
	 */
	void EnableCustomDepth(bool bEnable);

	// �����Ƿ����ٵ�������ʱ
	UPROPERTY(Replicated)
	bool bDestroyOnDrop = false;

	// �����Ƿ�ᱻ����
	UPROPERTY(Replicated)
	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFiringType FiringType;	// �������

	/*
	 * ���ٺ�ɢ��
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;		// ��������

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.0f;			// ����뾶


	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;	// �Ƿ�ʹ��ɢ��

public:
	// Sets default values for this actor's properties
	AWeapon();
	virtual void OnRep_Owner() override;
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	void SetHUDAmmo();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	// �������� / ������������
	virtual void Dropped();
	void AddAmmo(int32 Amount);

	FVector TraceEndWithScatter(const FVector& HitTarget) const;	// ���ٽ���λ��


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();

	virtual void OnEquipped();

	virtual void OnDropped();

	virtual void OnEquippedSecondary();

	// ������������
	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	// �����Ŀ�����
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,	// ԭʼ���
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
	void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,	// ԭʼ���
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;	// �����˺�

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 50.f;	// ��ͷ�˺�

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = true;		// �Ƿ�ʹ�÷������˵���

	UFUNCTION()
	void OnPingTooHigh(bool bHighPing);		// ���ӳ�ί�а󶨺���

private:
	// �����Ǽ�����
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	// ������ײ��
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	// ����״̬
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	// ����ʰȡ��Widget���
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	// ����״̬ͬ��
	UFUNCTION()
	void OnRep_WeaponState();

	// �������𶯻�
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	// ����׼��
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	// �����ӵ�����
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	// �۳���ҩ����������Ƿ���һ����Ч��������
	void SpendRound();

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 Amount);

	// δ����ķ�������ҩ��������
	// ��SpendRound()�����ӣ���ClientUpdateAmmo()�м���
	int32 Sequence = 0;

	// �����ĵ�ҩ����
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// ��������
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	// Called every frame

	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }

	bool IsEmpty();
	bool IsFull();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};
