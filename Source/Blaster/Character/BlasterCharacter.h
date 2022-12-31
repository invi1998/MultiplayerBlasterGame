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

	// ���ſ�����̫�涯��
	void PlayFireMontage(bool bAiming);

	// �����ܻ�rpc�������ಥ��
	UFUNCTION(NetMulticast, Unreliable)
		void MuticastHit();

	virtual void OnRep_ReplicatedMovement() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ���ֲ���ӳ�亯��

	void MoveForward(float Value);

	void MoveRight(float Value);

	void Turn(float Value);

	void LookUp(float Value);

	// ��Eװ������
	void EquipButtonPressed();

	// ��ctrl�׷�������
	void CrouchButtonPressed();

	// ����Ҽ���׼
	void AimButtonPressed();

	// �ɿ��Ҽ���׼
	void AimButtonReleased();
	void CalculateAO_Pitch();

	// ����Ŀ��ƫ����
	void AimOffset(float DeltaTime);

	// ģ�����
	void SimProxiesTurn();

	// ԭ��Actor��jump�߼��ǣ��ڶ��µ�ʱ���ǲ�������Ծ�ģ�Ϊ��ʵ�ֶ��µ�ʱ�򣬰��ո����ȡ���׷���������Ҫ��дjump
	virtual void Jump() override;

	// ����������������£�
	void FireButtonPressed();
	// �������������̧��
	void FireButtonReleased();

	// ���������ܻ�����̫�涯��
	void PlayHitReactMontage();

private:
	// ��ӵ��ɱۺ������

	// ���Ժ꣬�������κεط��ɼ����뽫��������˹�Ϊһ�࣬�������
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	// ��Ȼ����Ҫ����ͼ�������ǵĲ�����������Ҫ������ͼֻ��
	// �����˽�б���������ͼֻ������ô����Ҫ����˽�˷��ʣ�����ζ�Ž����������¶����ͼ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;

	// �������ǰ������
	// ������Ҫ�����������������ζ�ŵ����ڷ������Ϸ����仯ʱ��������Ҫͬ���ڿͻ�����Ҳ���и��ģ����ǿ��Ը���ָ��������ָ��
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

	// ��ʼ��ת��Ϣ
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	// ��������Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;

	// �����ܻ������Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* HitReactMontage;

	// �����ɫ��̫������������ͷ
	void HidCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
		float CameraThreshold = 200.f;

	// �Ƿ���ת������
	bool bRotateRootBone;

	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/*
	 * ���Ѫ��
	 */

	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;

	// ���Ѫ�����ɱ�Ѫ��֪ͨ��������
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
		float Health = 100.f;

	// �����Ѫ�����¸��ͻ���
	UFUNCTION()
		void OnRep_Health();

	class ABlasterPlayerController* BlasterPlayerController;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	// �Ƿ�װ������
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
