// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ���Ե�����ײλ�ã���ʱ��ײ����ʧ�ܣ�Ҳ��������Spawn
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// �����ɱ����ӵ�������
	CameraBoom->SetupAttachment(GetMesh());
	// ��������۳���
	CameraBoom->TargetArmLength = 600.f;
	// ���������Ϊtrue����������������������ʱ���Ϳ������ſ�������ת������ˡ�
	CameraBoom->bUsePawnControlRotation = true;

	// ǰ�����
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	// �������
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// ��Ϊ������ۿ�����ת�����Բ���Ҫ������תΪtrue
	FollowCamera->bUsePawnControlRotation = false;

	// ��ϣ����ɫ�������һ����ת
	bUseControllerRotationYaw = false;

	// ��ȡ��ɫ�ƶ������ʽ�ɫ�ƶ����
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// ս�����
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// ���ý�ɫ�ɶ׷���Ҳ������UE�༭���й�ѡ��
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// ���ý�ɫ�����岻���赲����������ù��������赲�����
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	// ���ý�ɫ��ת���ʣ�����ת�����Ȼ
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	// ���ý�ɫĬ�����ƣ���ת�䣩
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// ���ý�ɫ����Ƶ�ʺ���С����Ƶ��
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineCommponent"));
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}

void ABlasterCharacter::RotatePlace(float DeltaTime)
{
	if (bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		// �����һ�����ؽ�ɫ���Լ���
		AimOffset(DeltaTime);
	}
	else
	{
		// ����Ǵ����ɫ�������ͻ��˵Ľ�ɫ��
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		// ����ɫ�ܵ����˺���̬�󶨵����ǵĴ�����ReceiveDamage��
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotatePlace(DeltaTime);

	HidCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// �������������Ҫע��Ҫ�����ص����������ĵط�
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bDisableGamePlay);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("Rifle_Aim") : FName("Rifle_Hip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Rifle");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0;
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	// �����󶨵�����ִ�к���
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ABlasterCharacter::ReloadButtonPressed);
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGamePlay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGamePlay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGamePlay) return;

	if (Combat)
	{
		if (HasAuthority())
		{
			// ����Ƿ���˰���E���Ǿ�ֱ��ִ��ʰȡ�����Ķ���
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			// ������ǿͻ��ˣ��ͻ�����Ҫ����RTC
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGamePlay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

// �����ƶ����ʱ����ϣ����ɫ���ϰ벿����վ��״̬���ܰ�����ƶ�ƫ�������л������ܵĶ�����
// ��ƫ��������һ���޶ȣ�90�㣩����ϣ����ɫ����ת��
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// �����ɫû��װ����������ô���˳�
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	// ��ȡ��ɫ�ٶ�ʸ��
	float Spead = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// ����ɫ����վ��״̬������û����Ծ����أ�
	if (Spead == 0.0f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		// �����ɫû��ԭ��ת��������ô���ֵ��ֱ�Ӹ�ֵΪ AO_Yaw
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		// �����ɫ����ת��������ô����Ҫ�Ը�ֵ���в�ֵ������һ�����Է��� TurnInPlace ��ȥ��

		bUseControllerRotationYaw = true;
		// ԭ��ת����
		TurnInPlace(DeltaTime);
	}

	// �ܶ�������Ծ����أ�
	if (Spead > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		// �����ǵĽ�ɫ�ܲ������ڿ���ʱ����װ����������ǰ���£������ᱣ��ÿһ֡����ת��Ϣ
		// �����ת��Ϣ����Ϊһ��������Ϣ���ݸ���һ��ifʹ��
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// ͬʱҲӦ�ý�AO_Yaw�ÿ���0����Ϊ���ǻ���������м���
		AO_Yaw = 0.f;
		// ͬʱһ�����ǿ�ʼ�˶������Ǿ���Ҫ����ʹ�ÿ�������ת
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	// ��ȡ���ǲ�����
	AO_Pitch = GetBaseAimRotation().Pitch;
	//if (HasAuthority() && !IsLocallyControlled())
	//{
	//	// ����UE_LOG�鿴�ڷ���˳������Ǵ���bug������
	//	UE_LOG(LogTemp, Warning, TEXT("AO_Yaw : %f"), AO_Yaw);
	//	// ���Կ��������ǵĸ��ӽǵ�ͷ���ذ��ʱ�����Ƿ�����յ�������ȷʵ270-360֮������ݣ��ⲻ�������趨�е����ǵġ�-90,0��
	//}
	// !IsLocallyControlled() ����жϾ���ȷ�������ڱ������ϣ��Ǳ������ʹ�������RPC�����ݱ�ѹ��������Ҫ���л�ԭӳ��
	if (AO_Pitch > 90.F && !IsLocallyControlled())
	{
		// ���������ݴ� [270, 360) ӳ�䵽 [-90, 0)
		// �����ɫ�����޸���Ϊ�� CharacterMovementComponent�У���������GetPackedAngles����ȡ�������ݣ�
		// �������Ϊ�˽��������ݣ���ת���ݣ�ͨ��RPC���紫��ʱ��Ϊ�˼��ٿ��ռ�ã�������ѹ����4�ֽڣ�
		// �������ǿͻ��˵ĸ��ӽ��ڷ������ʾȷʵ̧ͷ�����ӽǵ�bug
		// �����ǵĳ����д�У�����ϰ�ߵĽǶȿ��������� -90��90����������������У���������ڻ�ȡ��Щ��ת�Ƕ�����ʱ��
		// ���Ὣ��ѹ��Ϊһ���޷��ŵ�������
		// ���ѹ�����̾���ͨ�� FRotator::CompressAxisToShort(Yaw) , FRotator::CompressAxisToShort(Patch)
		// �������������е�ѹ����������ԭ������
		///*FORCEINLINE uint32 UCharacterMovementComponent::PackYawAndPitchTo32(const float Yaw, const float Pitch)
		//{
		//	const uint32 YawShort = FRotator::CompressAxisToShort(Yaw);
		//	const uint32 PitchShort = FRotator::CompressAxisToShort(Pitch);
		//	const uint32 Rotation32 = (YawShort << 16) | PitchShort;
		//	return Rotation32;
		//}*/
		// Ȼ����� ������Ҫ��ִ�к��� CompressAxisToShort,ԭ������
		//template<typename T>
		//FORCEINLINE uint16 TRotator<T>::CompressAxisToShort(T Angle)
		//{
		//	// map [0->360) to [0->65536) and mask off any winding
		//	return FMath::RoundToInt(Angle * (T)65536.f / (T)360.f) & 0xFFFF;
		//}
		// ��Դ���п��Կ�����������������������ģ����ǽ�һ��float�ĽǶ����ݣ�������ֵ��Χ��[0,360)��
		// (Angle * (T)65536.f / (T)360.f)�⴮����õ��Ľ����һ������ [0, 65536)֮��ĸ���ֵ
		// RoundToInt���ǽ����������룬��������תΪ����
		// ����֪��float�����Ǿ���С���㾫�ȵģ���Щ������Ϣ����Ҫ�ֽ���Ϣ�����д洢�ģ�
		// ͨ����float����maskӳ�䵽�����Χ����ɾ����ЩС���㣬���Ǿ͵õ���һ��һ������ [0, 65536)֮�������ֵ
		// �� 65535 ���ֵ����ʮ�������������� 0xFFFF,16bitλ�����������
		// ��ǰ����������� 0xFFFF ���а�λ�� & ���㣬�õ��Ľ�����ǽ��������ĸ�������360������и
		// ��˼���ǣ������㴫�ݽ���361����ô�ھ�������󣬵õ��Ľ����ʵ����1����һ�ֵ���ת��ʵ������ת1����һ����Ч��
		// ���������õײ��ڵ���RPC������ת�Ǻ͸�����ʱ��Ϊ�˼������翪���������Ż�

		// ���ڣ����ǿ���֪�������ǵ���ת��ѹ��Ϊ [0,360)�ĸ�ʽ������ͨ��rpc���ú����ᱻ��ѹ������ԭ�ظ�������
		// ���������ﻹԭ����Ϊ����ԭʹ�õ���[0,360�������ݣ�����ʧ�����ǵĸ�����Ϣ����������������Ҫ��ӳ��
		// [270, 360) -> [-90, 0)
		// map pitch from [270,260) to [-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;

	float Spead = CalculateSpeed();
	if (Spead > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();

	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		// �ж��Ƿ���Ҫת��
		if (ProxyYaw > TurnThreshold)
		{
			// ����ת
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < TurnThreshold)
		{
			// ����ת
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bDisableGamePlay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// ��� AO_Yaw����90������Ӧ������ת��С��-90��Ӧ������ת
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// ��ֵ�������һ���������������ת�����ж��(ֵԽС������Խ����Խ�죩
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			// ���ԭ��ת�����15��,��ô�Ͳ��ٽ���ת��
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

void ABlasterCharacter::HidCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	// ��ȡ��ɫ�ٶ�ʸ��
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim();
	// ���ø����ʱ
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();

	// ��ʼ��ɫ�ܽ�Ч��
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// ���ý�ɫ�ƶ�����ֹWASD�����ƶ���
	// GetCharacterMovement()->DisableMovement();
	// ����ֹͣ�ƶ� (��ֹ��ת������������ת��ȣ�
	// GetCharacterMovement()->StopMovementImmediately();
	// ���ÿ���
	bDisableGamePlay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// ������ײ
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ����Elim Bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();

	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;

	return Combat->CombatState;
}
