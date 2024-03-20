// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Shotgun.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 这些变量都是需要通过rpc网络复制到各个客户单进行同步的数据
	// 注册装备的武器,是否瞄准，携带的弹药数量
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);	// 装备的武器
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);	// 次要武器
	DOREPLIFETIME(UCombatComponent, bAiming);	// 是否瞄准
	// 携带的弹药数量只对客户端来说是有重要意义，因为他和客户端是一对一的关系，只有一个客户端需要将这个值设置在HUD中显示
	// 所以这里可以使用适当的生命周期条件,并指定条件对象 COND_OwnerOnly
	// 这样，该变量就只会复制到拥有该弹药的客户端，而不会广播给所有客户端，谁拿到，复制给谁，就这样，这将会节省带宽提高性能
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);	// 携带的弹药数量
	DOREPLIFETIME(UCombatComponent, CombatState); // 战斗状态
	DOREPLIFETIME(UCombatComponent, GrenadeCount);	// 手榴弹数量
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 Amount)
{
	if (Character == nullptr) return;

	if (CarriedAmmoMap.Contains(WeaponType))	// 确保携带的弹药数量中包含该类型的弹药
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + Amount, 0, MaxCarriedAmmoMap[WeaponType]);	// 更新携带的弹药数量
		// CarriedAmmo = CarriedAmmoMap[WeaponType];	// 更新携带的弹药数量
		UpdateCarriedAmmo();	// 更新携带的弹药数量
	}

	// 同时，如果我们拾取了弹药，这时我们装备的武器正好是空的，而且也正好是我们拾取的弹药类型，那么我们就需要换弹
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

// Called when the game starts or when spawned
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, MaxARAmmo);	// 最大携带的突击步枪弹药数量
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, MaxRocketAmmo);	// 最大携带的火箭弹药数量
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, MaxPistolAmmo);	// 最大携带的手枪子弹的数量
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, MaxSMGAmmo);	// 最大携带的冲锋枪子弹的数量
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, MaxShotgunAmmo);	// 最大携带的霰弹枪子弹的数量
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, MaxSniperAmmo);	// 最大携带的狙击枪子弹的数量
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, MaxGrenadeLauncherAmmo);	// 最大携带的手榴弹的数量

	// ...
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (Character->HasAuthority()) {
			InitializeCarriedAmmo();
		}
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	// 战斗组件需要知道按下和抬起开火按钮的时间
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValue();
	}
	
}

void UCombatComponent::JumpToShotgunEnd()
{
	// 跳转到霰弹枪装弹结束动画蒙太奇部分（ShotgunEnd）
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();

	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
	
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		// 获取手榴弹组件的位置，然后记录该位置，用于后续在该位置生成一个手榴弹
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();

		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams);
		}
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->FiringType)
			{
			case EFiringType::EFT_HitScan:
				FireHitscanWeapon();
				break;
			case EFiringType::EFT_Projectile: 
				FireProjectileWeapon();
				break;
			case EFiringType::EFT_Shotgun: 
				FireShotgun();
				break;
			case EFiringType::EFT_MAX: break;
			default: break;
			}

		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 如果武器是使用散射的，那么就使用散射
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;

		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireHitscanWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 如果武器是使用散射的，那么就使用散射
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		
		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}

		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceHitWithScatter(HitTarget, HitTargets);

		if (!Character->HasAuthority())
		{
			LocalShotgunFire(HitTargets);
		}
		
		ServerShotgunFire(HitTargets);
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Character == nullptr || Shotgun == nullptr) return;

	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	// 因为霰弹枪一次性装弹会有4次装弹动画，所以这里特写霰弹枪的开火判定逻辑，只要有一次装弹（有子弹，哪怕是在装弹状态下也可以开火）
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		return true;
	}

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) 
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading && EquippedWeapon != nullptr && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CarriedAmmo == 0;

	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bAiming = bIsAiming;
	// 如果是客户端，那么客户端需要通过RPC网络复制调用服务端的函数进行瞄准
	// 当然，因为RPC只会在客户端进行调用，所以，这里不用判断是客户端还是服务端
	ServerSetAiming(bIsAiming);
	//if (!Character->HasAuthority())
	//{
	//	// 如果是客户端，那么客户端需要通过RPC网络复制调用服务端的函数进行瞄准
	//	// 当然，因为RPC只会在客户端进行调用，所以，这里不用判断是客户端还是服务端
	//	ServerSetAiming(bIsAiming);
	//}
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}

	ReloadEmptyWeapon();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TracerHitTarget)
{
	MuticastFire(TracerHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;	// 如果是客户端，那么客户端不需要再次进行多播，因为客户端已经在本地进行了多播

	LocalShotgunFire(TraceHitTargets);	// 如果是服务端，那么服务端需要再次进行多播，因为服务端需要将多播的结果同步到客户端
}

void UCombatComponent::MuticastFire_Implementation(const FVector_NetQuantize& TracerHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	
	LocalFire(TracerHitTarget);
	
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		// 如果当前角色主武器装备，次要武器没有装备，那么就装备次要武器
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		// 如果当前角色主武器没有装备，或者主幅武器都装备了，那么就装备主武器
		EquipPrimaryWeapon(WeaponToEquip);
	}

	// 一旦bOrientRotationToMovement设置为True后，角色的朝向会转向移动的方向
	// 一旦bOrientRotationToMovement 为False，但是bUseControllerDesiredRotation设置为True。
	// 那么在角色移动中，如果Controller的朝向和角色朝向不一致，
	// 角色朝向就会平滑的按照RotationRate里配置的各个轴的旋转速率来旋转角色的Rotation到Controller的朝向。
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SwapWeapon()
{
	if (Character == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon && SecondaryWeapon)
	{
		// 如果当前角色主武器装备，次要武器也装备了，那么就交换主武器和次要武器
		AWeapon* TempWeapon = EquippedWeapon;
		EquippedWeapon = SecondaryWeapon;
		SecondaryWeapon = TempWeapon;

		// 交换武器后，需要将武器的状态设置为装备状态
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

		// 交换武器后，需要将武器的位置设置为装备状态
		AttachActorToRightHand(EquippedWeapon);
		AttachActorToBack(SecondaryWeapon);

		// 交换武器后，需要将武器的拥有者设置为角色
		EquippedWeapon->SetOwner(Character);
		SecondaryWeapon->SetOwner(Character);

		//// 交换武器后，需要将武器的HUD弹药数量设置
		EquippedWeapon->SetHUDAmmo();
		//SecondaryWeapon->SetHUDAmmo();

		// 交换武器后，需要将武器的携带的弹药数量设置
		UpdateCarriedAmmo();

		// 交换武器后，需要将武器的自定义深度设置
		EquippedWeapon->EnableCustomDepth(false);
		SecondaryWeapon->EnableCustomDepth(false);

		// 交换武器后，需要将武器的装备声音设置
		PlayEquipWeaponSound(EquippedWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);

		// 交换武器后，需要将武器的装备声音设置
		ReloadEmptyWeapon();
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip) return;

	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();

	PlayEquipWeaponSound(WeaponToEquip);

	ReloadEmptyWeapon();

	EquippedWeapon->EnableCustomDepth(false);
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip) return;

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

	AttachActorToBack(SecondaryWeapon);

	SecondaryWeapon->SetOwner(Character);

	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}

}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;

	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName(bUsePistolSocket ? "PistolSocket" : "LeftHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));

	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{

	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (Character && EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		// 如果当前角色携带的弹药数量小于等于0（没有子弹），那么就没必要rpc广播换弹动作，没有子弹，不需要换弹，节省网络带宽
		ServerReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValue();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Laset = FMath::Min(RoomInMag, AmmountCarried);
		return FMath::Clamp(RoomInMag, 0, Laset);
	}
	return 0;
}

// 服务端换弹
void UCombatComponent::ThrowGrenade()
{
	if (GrenadeCount <= 0) return;	// 如果手榴弹数量小于等于0，那么就不需要进行投掷手榴弹的动作了

	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;;

	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (Character)
	{
		Character->PlayThrowGrenadeMontage();	// 播放投掷手榴弹的蒙太奇动画
		AttachActorToLeftHand(EquippedWeapon);	// 将武器附加到左手
		ShowAttachedGrenade(true);		// 显示手榴弹
	}
	if (Character && !Character->HasAuthority())	// 如果是客户端，那么客户端需要通过RPC网络复制调用服务端的函数进行投掷手榴弹
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())	// 如果是服务端，那么服务端需要通过RPC网络复制调用客户端的函数进行投掷手榴弹
	{
		GrenadeCount = FMath::Clamp(GrenadeCount - 1, 0, GrenadeCount);	// 手榴弹数量减1
		UpdateGrenadeCount();
	}
	
}

// 服务端投掷手榴弹
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (GrenadeCount <= 0) return;	// 如果手榴弹数量小于等于0，那么就不需要进行投掷手榴弹的动作了
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	GrenadeCount = FMath::Clamp(GrenadeCount - 1, 0, GrenadeCount);	// 手榴弹数量减1
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	default:
		break;
	}
}

void UCombatComponent::UpdateAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(1);

	bCanFire = true;

	// 如果霰弹枪已经填充满子弹
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}

}

void UCombatComponent::OnRep_GrenadeCount()
{
	UpdateGrenadeCount();
}

void UCombatComponent::UpdateGrenadeCount()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;	// 获取控制器

	if (Controller)
	{
		Controller->SetHUDGrenades(GrenadeCount);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		AttachActorToRightHand(EquippedWeapon);
		// 如果我们装备了武器，那么我们就不应该继续定向移动
		// （通过是否装备武器来解决客户端定向移动设置问题，避免添加额外的RPC变量的开销）
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		PlayEquipWeaponSound(EquippedWeapon);

		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBack(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
		
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// 获取视口大小
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 射击准心位置
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// 将屏幕坐标转化为世界坐标
	FVector CrosshairWorldPosition;		// 准心世界坐标向量
	FVector CrosshairWorldDirection;	// 准心世界方向向量
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	// 判断坐标是否转化成功
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}

		// 如果没有跟踪到任何结果（比如瞄准天空，瞄准的物品超出80000等)
		// 这种情况下，我们就将这个结果设置为End向量
		if (!TraceHitResult.bBlockingHit)
		{
			// 如果目标没有命中
			TraceHitResult.ImpactPoint = End;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// 计算十字准心的伸展值

			// 速度映射[0, 600] -> [0. 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMutiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFacor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMutiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFacor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}
