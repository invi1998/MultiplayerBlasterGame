// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerStart/TeamPlayerStart.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 尝试调整碰撞位置，及时碰撞调整失败，也依旧生成Spawn
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// 将弹簧臂链接到网格上
	CameraBoom->SetupAttachment(GetMesh());
	// 设置相机臂长度
	CameraBoom->TargetArmLength = 600.f;
	// 把这个设置为true，这样当我们添加鼠标输入时，就可以随着控制器旋转相机吊杆。
	CameraBoom->bUsePawnControlRotation = true;

	// 前置相机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	// 绑定相机臂
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// 因为有相机臂控制旋转，所以不需要设置旋转为true
	FollowCamera->bUsePawnControlRotation = false;

	// 不希望角色与控制器一起旋转
	bUseControllerRotationYaw = false;

	// 获取角色移动来访问角色移动组件
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// 战斗组件
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// BUFF组件
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);	// 设置BUFF组件为可复制

	// 服务端倒带组件
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	// 设置角色可蹲伏（也可以在UE编辑器中勾选）
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// 设置角色胶囊体不会阻挡摄像机，设置骨骼不会阻挡摄像机
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	// 设置角色旋转速率，让其转身更自然
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	// 设置角色默认姿势（不转弯）
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// 设置角色更新频率和最小更新频率
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineCommponent"));

	// 手榴弹组件
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	// 将手榴弹组件绑定到右手的GrenadeSocket插槽上
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	// 设置手榴弹的碰撞
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*
	 * 受击盒子 用于服务端倒带算法
	 */
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (const auto& HitCollisionBox : HitCollisionBoxes)
	{
		if (HitCollisionBox.Value == nullptr) continue;

		HitCollisionBox.Value->SetCollisionObjectType(ECC_HitBox);		// 设置碰撞对象类型，这是我们自定义的碰撞通道
		HitCollisionBox.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// 设置碰撞响应，忽略所有通道
		HitCollisionBox.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// 设置碰撞响应，只响应我们自定义的碰撞通道
		HitCollisionBox.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 设置碰撞启用，不启用碰撞
	}

	// ImpulseCharacter();	// 给角色添加冲量
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDHealthNative()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealthNative(Health, MaxHealth, BeforeDamageHealth, AfterHealHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
		BlasterPlayerController->SetHUDShieldNative(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		if (Combat && Combat->EquippedWeapon)
		{
			BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);	// 设置手榴弹数量
			BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());	// 设置武器弹药数量
		}
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			OnPlayerInitialized();

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheCrown();
			}
		}
	}
}

void ABlasterCharacter::SetSpawnPoint()
{
	if (HasAuthority() && BlasterPlayerState && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;	// 玩家出生点数组
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);	// 获取所有的玩家出生点

		TArray<ATeamPlayerStart*> TeamPlayerStarts;	// 队伍玩家出生点数组
		for (AActor* PlayerStart : PlayerStarts)
		{
			ATeamPlayerStart* TeamPlayerStart = Cast<ATeamPlayerStart>(PlayerStart);
			if (TeamPlayerStart && TeamPlayerStart->GetTeam() == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamPlayerStart);		// 将相同队伍的玩家出生点添加到队伍玩家出生点数组中
			}
		}

		if (TeamPlayerStarts.Num() > 0)
		{
			const int32 RandomIndex = FMath::RandRange(0, TeamPlayerStarts.Num() - 1);	// 随机索引

			// 打印RandomIndex到屏幕上
			UKismetSystemLibrary::PrintString(this, FString::FromInt(RandomIndex), true, false, FLinearColor::Red, 0.f);

			const FVector SpawnLocation = TeamPlayerStarts[RandomIndex]->GetActorLocation();	// 随机生成的位置
			const FRotator SpawnRotation = TeamPlayerStarts[RandomIndex]->GetActorRotation();	// 随机生成的旋转
			SetActorLocation(SpawnLocation);	// 设置角色位置
			SetActorRotation(SpawnRotation);	// 设置角色旋转
		}
	}
}

void ABlasterCharacter::OnPlayerInitialized()
{
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColor(BlasterPlayerState->GetTeam());
	SetSpawnPoint();
}

void ABlasterCharacter::RotatePlace(float DeltaTime)
{
	if (Combat && Combat->bHoldingFlag)
	{
		bUseControllerRotationYaw = false;		// 不使用控制器旋转Yaw
		GetCharacterMovement()->bOrientRotationToMovement = true;	// 朝向移动方向
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;	// 不转弯
		return;
	}
	if (Combat && Combat->EquippedWeapon)
	{
		bUseControllerRotationYaw = false;		// 不使用控制器旋转Yaw
		GetCharacterMovement()->bOrientRotationToMovement = true;	// 朝向移动方向
	}
	if (bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		// 如果是一个本地角色（自己）
		AimOffset(DeltaTime);
	}
	else
	{
		// 如果是代理角色（其他客户端的角色）
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	UWorld* World = GetWorld();
	if (World && !bElimmed && BlasterGameMode && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;	// 对于角色的默认武器，我们希望它在被丢弃时被销毁
		StartingWeapon->bDestroyOnDrop = true;	// 对于角色的默认武器，我们希望它在被丢弃时被销毁
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
		
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	const bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::SetBacktrackingTime(float BacktrackingTime, float CostTime)
{
	if (LagCompensation)
	{
		LagCompensation->ServerBacktrackTime(BacktrackingTime, CostTime);
	}
}

void ABlasterCharacter::MulticastGainedTheCrown_Implementation()
{
	if (!CrowSystem) return;
	if (CrowComponent == nullptr)
	{
		CrowComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrowSystem, 
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,		// 保持世界位置
			false
			);
	}
	if (CrowComponent)
	{
		CrowComponent->Activate();	// 激活
	}
}

void ABlasterCharacter::MulticastLostTheCrown_Implementation()
{
	if (CrowComponent)
	{
		CrowComponent->DestroyComponent();	// 销毁组件
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || DefaultMaterialInstance == nullptr) return;

	switch (Team)
	{
		case ETeam::ET_NoTeam:
			GetMesh()->SetMaterial(0, DefaultMaterialInstance);	// 设置默认材质
			DissolveMaterialInstance = DefaultDissolveMaterialInstance;		// 设置溶解材质
			break;
		case ETeam::ET_RedTeam:
			GetMesh()->SetMaterial(0, RedMaterialInstance);		// 设置红队材质
			DissolveMaterialInstance = RedDissolveMaterialInstance;			// 设置红队溶解材质
			break;
		case ETeam::ET_BlueTeam:
			GetMesh()->SetMaterial(0, BlueMaterialInstance);		// 设置蓝队材质
			DissolveMaterialInstance = BlueDissolveMaterialInstance;		// 设置蓝队溶解材质
			break;
		default: break;
	}
}

void ABlasterCharacter::ImpulseCharacter()
{
	if (GetCharacterMovement() == nullptr) return;

	// 计算角色的速度
	FVector Impulse = GetActorForwardVector() * 1000.f;
	// 添加冲量
	GetCharacterMovement()->AddImpulse(Impulse, true);
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();	// 角色生成默认武器

	UpdateHUDAmmo();		// 更新HUD弹药数量
	UpdateHUDHealth();
	UpdateHUDShield();

	BeforeDamageHealth = Health;
	AfterHealHealth = 0.f;
	UpdateHUDHealthNative();

	if (HasAuthority())
	{
		// 将角色受到的伤害动态绑定到我们的处理函数ReceiveDamage上
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotatePlace(DeltaTime);

	DamageRampUp(DeltaTime);

	HidCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 这里就是我们需要注册要复制重叠武器变量的地方
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);	// 只有所有者才会复制,这样就不会出现武器重叠的bug了
	DOREPLIFETIME(ABlasterCharacter, Health);	// 复制血量
	DOREPLIFETIME(ABlasterCharacter, BeforeDamageHealth);	// 复制血量
	DOREPLIFETIME(ABlasterCharacter, AfterHealHealth);	// 复制血量
	DOREPLIFETIME(ABlasterCharacter, bDisableGamePlay);	// 复制是否禁用游戏
	DOREPLIFETIME(ABlasterCharacter, Shield);	// 复制护盾
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);

		Buff->SetInitialJumpZVelocity(GetCharacterMovement()->JumpZVelocity);	// 设置初始跳跃速度
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
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
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		default: break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlaySwapWeaponMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
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

void ABlasterCharacter::PlayHitReactMontage() const
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

void ABlasterCharacter::PlayThrowGrenadeMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许扔手榴弹
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
                                      AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;	// 如果已经被淘汰，那么就不再接受伤害

	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	if (BlasterGameMode)
	{
		Damage = BlasterGameMode->CalculateDamage(Controller, InstigatorController, Damage);

		float DamageToHealth = Damage;	// 伤害值
		if (Shield > 0.f)
		{
			// 如果护盾大于0，那么就先扣除护盾
			const float DamageToShield = FMath::Clamp(Damage, 0.f, Shield);
			Shield -= DamageToShield;
			DamageToHealth = FMath::Clamp(Damage - DamageToShield, 0.f, Health);
		}

		BeforeDamageHealth = Health;
		Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
		DamageRate = DamageToHealth / 1.f;	// 伤害速率

		UpdateHUDHealthNative();
		UpdateHUDHealth();
		UpdateHUDShield();
		PlayHitReactMontage();

		if (Health == 0.f)
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

	// 按键绑定到操作执行函数
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Released, this, &ABlasterCharacter::GrenadeButtonPressed);
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
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许切换武器

		if (Combat->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerEquipButtonPressed();
		}
		
		if (Combat->IsValidSwapWeapon() && !HasAuthority() && Combat->CombatState == ECombatState::ECS_Unoccupied && OverlappingWeapon == nullptr)
		{
			PlaySwapWeaponMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapon;
			bFinishedSwapping = false;
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGamePlay) return;
	if (Combat && Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许切换是否蹲伏
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
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许重新装填
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许瞄准
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许瞄准
		Combat->SetAiming(false);
	}
}

// 当我移动鼠标时，我希望角色的上半部分在站立状态下能按鼠标移动偏移量进行环顾四周的动作，
// 当偏移量超过一定限度（90°），我希望角色进行转向
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// 如果角色没有装备武器，那么就退出
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	// 获取角色速度矢量
	float Spead = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// 当角色处于站立状态，并且没有跳跃（离地）
	if (Spead == 0.0f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		// 如果角色没有原地转身动作，那么这个值就直接赋值为 AO_Yaw
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		// 如果角色有做转身动作，那么就需要对该值进行差值处理，这一步可以放在 TurnInPlace 中去做

		bUseControllerRotationYaw = true;
		// 原地转弯检测
		TurnInPlace(DeltaTime);
	}

	// 跑动或者跳跃（离地）
	if (Spead > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		// 当我们的角色跑步或者在空中时（当装备了武器的前提下），他会保存每一帧的旋转信息
		// 这个旋转信息将作为一个增量信息传递给上一个if使用
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// 同时也应该将AO_Yaw置空清0，因为我们会在上面进行计算
		AO_Yaw = 0.f;
		// 同时一旦我们开始运动，我们就需要继续使用控制器旋转
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	// 获取仰角并设置
	AO_Pitch = GetBaseAimRotation().Pitch;
	//if (HasAuthority() && !IsLocallyControlled())
	//{
	//	// 利用UE_LOG查看在服务端出现仰角错误bug的数据
	//	UE_LOG(LogTemp, Warning, TEXT("AO_Yaw : %f"), AO_Yaw);
	//	// 可以看到，我们的俯视角低头看地板的时候，我们服务端收到的数据确实270-360之间的数据，这不是我们设定中的我们的【-90,0）
	//}
	// !IsLocallyControlled() 这个判断就是确保不是在本机器上，非本机器就代表经过了RPC，数据被压缩过，需要进行还原映射
	if (AO_Pitch > 90.F && !IsLocallyControlled())
	{
		// 将仰角数据从 [270, 360) 映射到 [-90, 0)
		// 这个映射将会修复因为在 CharacterMovementComponent中，这个组件在GetPackedAngles（获取仰角数据）
		// 这个函数为了将仰角数据（旋转数据）通过RPC网络传输时，为了减少宽带占用，将数据压缩到4字节，
		// 导致我们客户端的俯视角在服务端显示确实抬头的仰视角的bug
		// 在我们的程序编写中，我们习惯的角度控制是例如 -90到90，但是在虚幻引擎中，这个函数在获取这些旋转角度数据时，
		// 他会将其压缩为一个无符号的整形数
		// 这个压缩过程就是通过 FRotator::CompressAxisToShort(Yaw) , FRotator::CompressAxisToShort(Patch)
		// 这两个函数进行的压缩处理，函数原型如下
		///*FORCEINLINE uint32 UCharacterMovementComponent::PackYawAndPitchTo32(const float Yaw, const float Pitch)
		//{
		//	const uint32 YawShort = FRotator::CompressAxisToShort(Yaw);
		//	const uint32 PitchShort = FRotator::CompressAxisToShort(Pitch);
		//	const uint32 Rotation32 = (YawShort << 16) | PitchShort;
		//	return Rotation32;
		//}*/
		// 然后关于 这里主要的执行函数 CompressAxisToShort,原型如下
		//template<typename T>
		//FORCEINLINE uint16 TRotator<T>::CompressAxisToShort(T Angle)
		//{
		//	// map [0->360) to [0->65536) and mask off any winding
		//	return FMath::RoundToInt(Angle * (T)65536.f / (T)360.f) & 0xFFFF;
		//}
		// 从源码中可以看出来，这个函数这里所做的，就是将一个float的角度数据，他的数值范围是[0,360)，
		// (Angle * (T)65536.f / (T)360.f)这串计算得到的结果是一个介于 [0, 65536)之间的浮点值
		// RoundToInt则是进行四舍五入，将浮点数转为整形
		// 我们知道float数据是具有小数点精度的，这些精度信息是需要字节信息来进行存储的，
		// 通过将float数据mask映射到这个范围，并删除这些小数点，我们就得到了一个一个介于 [0, 65536)之间的整形值
		// 而 65535 这个值他的十六进制数正好是 0xFFFF,16bit位的最大整形数
		// 将前面的整形数与 0xFFFF 进行按位与 & 运算，得到的结果就是将传递来的浮点数按360°进行切割。
		// 意思就是，假设你传递进来361，那么在经过计算后，得到的结果其实就是1，这一轮的旋转其实和你旋转1°是一样的效果
		// 这个就是虚幻底层在调用RPC传输旋转角和俯仰角时，为了减少网络开销所做的优化

		// 现在，我们可以知道，我们的旋转被压缩为 [0,360)的格式，当它通过rpc调用后，他会被解压缩，还原回浮点数。
		// 这点就是这里还原，因为它还原使用的是[0,360）的数据，它丢失了我们的负数信息，所以这里我们需要做映射
		// [270, 360) -> [-90, 0)
		// map pitch from [270,260) to [-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);	// 映射
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
		// 判断是否需要转体
		if (ProxyYaw > TurnThreshold)
		{
			// 向右转
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < TurnThreshold)
		{
			// 向左转
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;	// 不转体
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bDisableGamePlay) return;
	if (Combat && Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许跳跃
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
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许开火
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		if (Combat->bHoldingFlag) return;	// 如果角色正在持有旗帜，那么就不允许开火
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
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->IsValidSwapWeapon())
		{
			Combat->SwapWeapon();
		}
		
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// 如果 AO_Yaw大于90，我们应该向右转。小于-90，应该向左转
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
		// 差值处理，最后一个参数决定了你的转身动作有多快(值越小，动作越灵敏越快）
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			// 如果原地转弯低于15°,那么就不再进行转弯
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
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;	// 隐藏主武器
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;	// 隐藏副武器
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	// 获取角色速度矢量
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();	// 更新HUD血量
	if (Health < LastHealth)
	{
		PlayHitReactMontage();	// 播放受伤动画
	}
}

void ABlasterCharacter::OnRep_BeforeHealth(float LastHealth)
{
	UpdateHUDHealthNative();	// 更新HUD血量
}

void ABlasterCharacter::OnRep_AfterHealth(float LastHealth)
{
	UpdateHUDHealthNative();	// 更新HUD血量
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();	// 更新HUD护盾
	if (Shield < LastShield)
	{
		PlayHitReactMontage();	// 播放受伤动画
	}
}

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapon();

	MulticastElim(bPlayerLeftGame);
}

void ABlasterCharacter::DropOrDestroyWeapon()
{
	if (Combat && Combat->EquippedWeapon)
	{
		DropOrDestroyWeapon(Combat->EquippedWeapon);
	}
	if (Combat && Combat->SecondaryWeapon)
	{
		DropOrDestroyWeapon(Combat->SecondaryWeapon);
	}
	if (Combat && Combat->TheFlag)
	{
		Combat->TheFlag->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (Weapon->bDestroyOnDrop || Weapon->bDestroyWeapon)
		{
			Weapon->Destroy();
		}
		else
		{
			Weapon->Dropped();
		}
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();

	// 开始角色溶解效果
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// 禁用角色移动（阻止WASD按键移动）
	// GetCharacterMovement()->DisableMovement();
	// 立即停止移动 (阻止旋转鼠标带来的人物转体等）
	// GetCharacterMovement()->StopMovementImmediately();
	// 禁用开火
	bDisableGamePlay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// 禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 禁用手雷碰撞

	// 生成Elim Bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	const bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	if (CrowComponent)
	{
		CrowComponent->DestroyInstance();
	}

	// 设置复活倒计时
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
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

void ABlasterCharacter::DamageRampUp(float DeltaTime)
{
	const float DamageThisFrame = DamageRate * DeltaTime;	// 计算本帧伤害量
	BeforeDamageHealth = FMath::Clamp(BeforeDamageHealth - DamageThisFrame, Health, MaxHealth);	// 计算伤害后的血量
	UpdateHUDHealthNative();	// 更新HUD血量

	if (BeforeDamageHealth <= Health)
	{
		DamageRate = 0.f;
	}
}

void ABlasterCharacter::ServerLeftGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
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

bool ABlasterCharacter::IsLocalReloading() const
{
	if (Combat == nullptr) return false;

	return Combat->bLocalReloading;;
}

bool ABlasterCharacter::IsHoldingFlag() const
{
	if (Combat == nullptr) return false;

	return Combat->bHoldingFlag;
}

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;

	if (BlasterPlayerState)
	{
		return BlasterPlayerState->GetTeam();
	}
	return ETeam::ET_NoTeam;
}

void ABlasterCharacter::SetIsHoldingFlag(bool bCond)
{
	if (Combat)
	{
		Combat->bHoldingFlag = bCond;
	}
}
