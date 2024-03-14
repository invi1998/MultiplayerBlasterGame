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
	// ��Щ�������ǿ������ڿ��ƴ���������ײ�������������ĵط�

	EWS_MAX UMETA(DisplayName = "DefaultMax")				// Ĭ�������
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
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
		int32 Ammo;

	// �����ӵ�����ͬ��
	UFUNCTION()
		void OnRep_Ammo();

	// �۳���ҩ����������Ƿ���һ����Ч��������
	void SpendRound();

	// �����ĵ�ҩ����
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
	bool IsFull();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
