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

	// ���ſ�����̫�涯��
	void PlayFireMontage(bool bAiming);

	// ���ſ�����̫�涯��
	void PlayReloadMontage();

	// ��������������̫�涯��
	void PlayElimMontage();

	// ���������ܻ�����̫�涯��
	void PlayHitReactMontage();

	// ����Ͷ�����񵯵���̫�涯��
	void PlayThrowGrenadeMontage();

	//// �����ܻ�rpc�������ಥ��
	//UFUNCTION(NetMulticast, Unreliable)
	//	void MuticastHit();

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
		bool bDisableGamePlay = false;

	// ��ʾ�ѻ���ǹ����׼��	ui
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();	// ����HUDѪ��

	void UpdateHUDShield();	// ����HUD����

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

	void ReloadButtonPressed();	// ��R����

	// ����Ҽ���׼
	void AimButtonPressed();

	// �ɿ��Ҽ���׼
	void AimButtonReleased();
	void CalculateAO_Pitch();	// ����Ŀ��ƫ���� pitch ��

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
	

	// ����Ͷ����ť���£�T��
	void GrenadeButtonPressed();

	// ��ɫ�ܵ��˺�������
	UFUNCTION()
		void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// ��ʼ��HUD
	void PollInit();

	// ԭ����ת
	void RotatePlace(float DeltaTime);

private:
	// ��ӵ��ɱۺ������

	// ���Ժ꣬�������κεط��ɼ����뽫��������˹�Ϊһ�࣬�������
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;	// ����

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;	// �����

	// ��Ȼ����Ҫ����ͼ�������ǵĲ�����������Ҫ������ͼֻ��
	// �����˽�б���������ͼֻ������ô����Ҫ����˽�˷��ʣ�����ζ�Ž����������¶����ͼ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;	// ����С����

	// �������ǰ������
	// ������Ҫ�����������������ζ�ŵ����ڷ������Ϸ����仯ʱ��������Ҫͬ���ڿͻ�����Ҳ���и��ģ����ǿ��Ը���ָ��������ָ��
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);	// ��OverlappingWeapon�����仯ʱ�����øú���

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
		class UCombatComponent* Combat;		// ս�����

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;		// buff���

	UFUNCTION(Server, Reliable)
		void ServerEquipButtonPressed();

	float AO_Yaw;	// Ŀ��ƫ���� yaw ��
	float InterpAO_Yaw;	// ��ֵĿ��ƫ���� yaw ��

	float AO_Pitch;	// Ŀ��ƫ���� pitch ��

	// ��ʼ��ת��Ϣ
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;		// ��ת״̬ 0������ת 1����ת 2����ת���

	void TurnInPlace(float DeltaTime);		// ��ת

	// ��������Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;

	// ����������̫�涯��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ReloadMontage;

	// �����ܻ������Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* HitReactMontage;

	// ��ɫ���������Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ElimMontage;

	// Ͷ�����񵯵Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ThrowGrenadeMontage;

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
		void OnRep_Health(float LastHealth);

	/*
	 * ��һ���
	 */
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY()
		class ABlasterPlayerController* BlasterPlayerController;

	// ����Ƿ񱻻��ܣ�������
	bool bElimmed = false;

	// ������������ʱʱ��
	FTimerHandle ElimTimer;

	// �������ʱ����ʱ�Ĵ����߼�
	void ElimTimerFinished();

	// ����ʱ��
	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.f;

	/*
	 * ��ɫ�ܽ�Ч��
	 */
	UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	UPROPERTY(EditAnywhere)
		UCurveFloat* DissolveCurve;

	// ����Ϸ����ʱ���ĵĶ�̬ʵ�������ʵ�������ｫ���ǲ���ʵ��
	// ��̬�ܽ���ʣ����ڴ洢��̬�����Ĳ���ʵ��
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// ���������ɫ��ͼ�Ĳ���ʵ��
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
	 * ����
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

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
