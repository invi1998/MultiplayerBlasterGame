// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);	// �뿪��Ϸί��

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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;		// ��������������

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// ��ȡ�������ڸ�������

	virtual void PostInitializeComponents() override;	// ��ʼ�����

	// ���ſ�����̫�涯��
	void PlayFireMontage(bool bAiming);

	// ���Ż�����̫�涯��
	void PlayReloadMontage();

	// ���Ž���������̫�涯��
	void PlaySwapWeaponMontage() const;

	// ��������������̫�涯��
	void PlayElimMontage();

	// ���������ܻ�����̫�涯��
	void PlayHitReactMontage() const;

	// ����Ͷ�����񵯵���̫�涯��
	void PlayThrowGrenadeMontage() const;

	//// �����ܻ�rpc�������ಥ��
	//UFUNCTION(NetMulticast, Unreliable)
	//	void MuticastHit();

	virtual void OnRep_ReplicatedMovement() override;

	void Elim(bool bPlayerLeftGame);		// ��ɫ������

	void DropOrDestroyWeapon();		// ��������������
	void DropOrDestroyWeapon(AWeapon* Weapon);		// ��������������

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);		// �ಥ����

	virtual void Destroyed() override;

	void SetAfterHealHealth(float Clamp) { AfterHealHealth = Clamp; }
	void SetBacktrackingTime(float BacktrackingTime, float CostTime);		// ����һ��ݵ�ָ��ʱ��

	UPROPERTY(Replicated)
	bool bDisableGamePlay = false;

	// ��ʾ�ѻ���ǹ����׼��	ui
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();	// ����HUDѪ��

	void UpdateHUDHealthNative();	// ����HUD�˺�

	void UpdateHUDShield();	// ����HUD����

	void UpdateHUDAmmo();	// ����HUD��ҩ

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;	// ���к���

	bool bFinishedSwapping = false;	// �Ƿ���ɽ���

	FOnLeftGame OnLeftGame;	// �뿪��Ϸί��

	UFUNCTION(Server, Reliable)
	void ServerLeftGame();	// ������뿪��Ϸ

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheCrown();	// �ಥ�������

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheCrown();	// �ಥʧȥ����

	UFUNCTION()
	void SetTeamColor(ETeam Team);	// ���ö�����ɫ

	void ImpulseCharacter();	// �ƿ���Χ�Ľ�ɫ

	void SetSpawnPoint();		// ���ó�����

	void OnPlayerInitialized();	// ��ҳ�ʼ��

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

	// ����Ĭ������
	void SpawnDefaultWeapon();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;		// ս�����

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;		// buff���

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;		// ����˵���������ӳٲ��������

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

	// ���������Ķ�����̫��
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapWeaponMontage;

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
	 * ���ڷ���˵��������к���
	 */
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* head;	// ͷ����

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* pelvis;	// �������

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* spine_02;	// ��׵����

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* spine_03;	// ��׵����

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* upperarm_l;	// �ϱۺ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* upperarm_r;	// �ϱۺ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* lowerarm_l;	// �±ۺ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* lowerarm_r;	// �±ۺ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* hand_l;	// �ֱۺ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* hand_r;	// �ֱۺ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* backpack;	// ����

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* blanket;	// ����̺��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* thigh_l;	// ���Ⱥ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* thigh_r;	// ���Ⱥ��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* calf_l;	// С�� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* calf_r;	// С�� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* foot_l;	// �ź��� ��

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* foot_r;	// �ź��� ��

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

	// �����Ѫ�����¸��ͻ���
	UFUNCTION()
	void OnRep_BeforeHealth(float LastHealth);

	UFUNCTION()
	void OnRep_AfterHealth(float LastHealth);

	/*
	 * ��һ���
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 0.f;

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
	 * ��ɫ�ܽ�Ч�� Elim Dissolve Effect
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
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/*
	 * ������ɫ
	 */
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMaterialInstance;		// ����ܽ����ʵ��

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterialInstance;		// ��Ӳ���ʵ��

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMaterialInstance;	// �����ܽ����ʵ��

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterialInstance;		// ���Ӳ���ʵ��

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DefaultDissolveMaterialInstance;		// Ĭ���ܽ����ʵ��

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DefaultMaterialInstance;		// Ĭ�ϲ���ʵ��

	/*
	 * ��������
	 */
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrowSystem;

	UPROPERTY()
	UNiagaraComponent* CrowComponent;

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

	/*
	 * ��ɫĬ������
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	/*
	 * ��ѪЧ��
	 */
	UPROPERTY(ReplicatedUsing = OnRep_BeforeHealth, VisibleAnywhere, Category = "Player Stats")
	float BeforeDamageHealth = 100.0f;	// ��Ѫǰ��Ѫ��

	/*
	 * ����Ч��
	 */
	UPROPERTY(ReplicatedUsing = OnRep_AfterHealth, VisibleAnywhere, Category = "Player Stats")
	float AfterHealHealth = 0.f;	// ���ƺ��Ѫ��

	bool bDamaging = false;		// �Ƿ����ڿ�Ѫ
	float DamageRate = 0.0f;	// ��Ѫ����
	float AmountToDamage = 0.0f;	// ��Ѫ��

	void DamageRampUp(float DeltaTime);	// ��Ѫ��������

	/*
	 * �뿪��Ϸ
	 */
	bool bLeftGame = false;	// �Ƿ��뿪��Ϸ

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;	// ��Ϸģʽ

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
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE void SetShield(float NewShield) { Shield = NewShield; }
	bool IsLocalReloading() const;
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	bool IsHoldingFlag() const;
	ETeam GetTeam();
};
