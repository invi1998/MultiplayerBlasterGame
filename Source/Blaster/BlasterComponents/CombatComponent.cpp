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

	// ��Щ����������Ҫͨ��rpc���縴�Ƶ������ͻ�������ͬ��������
	// ע��װ��������,�Ƿ���׼��Я���ĵ�ҩ����
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);	// װ��������
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);	// ��Ҫ����
	DOREPLIFETIME(UCombatComponent, bAiming);	// �Ƿ���׼
	// Я���ĵ�ҩ����ֻ�Կͻ�����˵������Ҫ���壬��Ϊ���Ϳͻ�����һ��һ�Ĺ�ϵ��ֻ��һ���ͻ�����Ҫ�����ֵ������HUD����ʾ
	// �����������ʹ���ʵ���������������,��ָ���������� COND_OwnerOnly
	// �������ñ�����ֻ�Ḵ�Ƶ�ӵ�иõ�ҩ�Ŀͻ��ˣ�������㲥�����пͻ��ˣ�˭�õ������Ƹ�˭�����������⽫���ʡ�����������
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);	// Я���ĵ�ҩ����
	DOREPLIFETIME(UCombatComponent, CombatState); // ս��״̬
	DOREPLIFETIME(UCombatComponent, GrenadeCount);	// ��������
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 Amount)
{
	if (Character == nullptr) return;

	if (CarriedAmmoMap.Contains(WeaponType))	// ȷ��Я���ĵ�ҩ�����а��������͵ĵ�ҩ
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + Amount, 0, MaxCarriedAmmoMap[WeaponType]);	// ����Я���ĵ�ҩ����
		// CarriedAmmo = CarriedAmmoMap[WeaponType];	// ����Я���ĵ�ҩ����
		UpdateCarriedAmmo();	// ����Я���ĵ�ҩ����
	}

	// ͬʱ���������ʰȡ�˵�ҩ����ʱ����װ�������������ǿյģ�����Ҳ����������ʰȡ�ĵ�ҩ���ͣ���ô���Ǿ���Ҫ����
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

// Called when the game starts or when spawned
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, MaxARAmmo);	// ���Я����ͻ����ǹ��ҩ����
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, MaxRocketAmmo);	// ���Я���Ļ����ҩ����
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, MaxPistolAmmo);	// ���Я������ǹ�ӵ�������
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, MaxSMGAmmo);	// ���Я���ĳ��ǹ�ӵ�������
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, MaxShotgunAmmo);	// ���Я��������ǹ�ӵ�������
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, MaxSniperAmmo);	// ���Я���ľѻ�ǹ�ӵ�������
	MaxCarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, MaxGrenadeLauncherAmmo);	// ���Я�������񵯵�����

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
	// ս�������Ҫ֪�����º�̧�𿪻�ť��ʱ��
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
	// ��ת������ǹװ������������̫�沿�֣�ShotgunEnd��
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
		// ��ȡ���������λ�ã�Ȼ���¼��λ�ã����ں����ڸ�λ������һ������
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
		// ���������ʹ��ɢ��ģ���ô��ʹ��ɢ��
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
		// ���������ʹ��ɢ��ģ���ô��ʹ��ɢ��
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
	// ��Ϊ����ǹһ����װ������4��װ������������������д����ǹ�Ŀ����ж��߼���ֻҪ��һ��װ�������ӵ�����������װ��״̬��Ҳ���Կ���
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
	// ����ǿͻ��ˣ���ô�ͻ�����Ҫͨ��RPC���縴�Ƶ��÷���˵ĺ���������׼
	// ��Ȼ����ΪRPCֻ���ڿͻ��˽��е��ã����ԣ����ﲻ���ж��ǿͻ��˻��Ƿ����
	ServerSetAiming(bIsAiming);
	//if (!Character->HasAuthority())
	//{
	//	// ����ǿͻ��ˣ���ô�ͻ�����Ҫͨ��RPC���縴�Ƶ��÷���˵ĺ���������׼
	//	// ��Ȼ����ΪRPCֻ���ڿͻ��˽��е��ã����ԣ����ﲻ���ж��ǿͻ��˻��Ƿ����
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
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;	// ����ǿͻ��ˣ���ô�ͻ��˲���Ҫ�ٴν��жಥ����Ϊ�ͻ����Ѿ��ڱ��ؽ����˶ಥ

	LocalShotgunFire(TraceHitTargets);	// ����Ƿ���ˣ���ô�������Ҫ�ٴν��жಥ����Ϊ�������Ҫ���ಥ�Ľ��ͬ�����ͻ���
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
		// �����ǰ��ɫ������װ������Ҫ����û��װ������ô��װ����Ҫ����
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		// �����ǰ��ɫ������û��װ������������������װ���ˣ���ô��װ��������
		EquipPrimaryWeapon(WeaponToEquip);
	}

	// һ��bOrientRotationToMovement����ΪTrue�󣬽�ɫ�ĳ����ת���ƶ��ķ���
	// һ��bOrientRotationToMovement ΪFalse������bUseControllerDesiredRotation����ΪTrue��
	// ��ô�ڽ�ɫ�ƶ��У����Controller�ĳ���ͽ�ɫ����һ�£�
	// ��ɫ����ͻ�ƽ���İ���RotationRate�����õĸ��������ת��������ת��ɫ��Rotation��Controller�ĳ���
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SwapWeapon()
{
	if (Character == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon && SecondaryWeapon)
	{
		// �����ǰ��ɫ������װ������Ҫ����Ҳװ���ˣ���ô�ͽ����������ʹ�Ҫ����
		AWeapon* TempWeapon = EquippedWeapon;
		EquippedWeapon = SecondaryWeapon;
		SecondaryWeapon = TempWeapon;

		// ������������Ҫ��������״̬����Ϊװ��״̬
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

		// ������������Ҫ��������λ������Ϊװ��״̬
		AttachActorToRightHand(EquippedWeapon);
		AttachActorToBack(SecondaryWeapon);

		// ������������Ҫ��������ӵ��������Ϊ��ɫ
		EquippedWeapon->SetOwner(Character);
		SecondaryWeapon->SetOwner(Character);

		//// ������������Ҫ��������HUD��ҩ��������
		EquippedWeapon->SetHUDAmmo();
		//SecondaryWeapon->SetHUDAmmo();

		// ������������Ҫ��������Я���ĵ�ҩ��������
		UpdateCarriedAmmo();

		// ������������Ҫ���������Զ����������
		EquippedWeapon->EnableCustomDepth(false);
		SecondaryWeapon->EnableCustomDepth(false);

		// ������������Ҫ��������װ����������
		PlayEquipWeaponSound(EquippedWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);

		// ������������Ҫ��������װ����������
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
		// �����ǰ��ɫЯ���ĵ�ҩ����С�ڵ���0��û���ӵ�������ô��û��Ҫrpc�㲥����������û���ӵ�������Ҫ��������ʡ�������
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

// ����˻���
void UCombatComponent::ThrowGrenade()
{
	if (GrenadeCount <= 0) return;	// �����������С�ڵ���0����ô�Ͳ���Ҫ����Ͷ�����񵯵Ķ�����

	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;;

	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (Character)
	{
		Character->PlayThrowGrenadeMontage();	// ����Ͷ�����񵯵���̫�涯��
		AttachActorToLeftHand(EquippedWeapon);	// ���������ӵ�����
		ShowAttachedGrenade(true);		// ��ʾ����
	}
	if (Character && !Character->HasAuthority())	// ����ǿͻ��ˣ���ô�ͻ�����Ҫͨ��RPC���縴�Ƶ��÷���˵ĺ�������Ͷ������
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())	// ����Ƿ���ˣ���ô�������Ҫͨ��RPC���縴�Ƶ��ÿͻ��˵ĺ�������Ͷ������
	{
		GrenadeCount = FMath::Clamp(GrenadeCount - 1, 0, GrenadeCount);	// ����������1
		UpdateGrenadeCount();
	}
	
}

// �����Ͷ������
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (GrenadeCount <= 0) return;	// �����������С�ڵ���0����ô�Ͳ���Ҫ����Ͷ�����񵯵Ķ�����
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	GrenadeCount = FMath::Clamp(GrenadeCount - 1, 0, GrenadeCount);	// ����������1
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

	// �������ǹ�Ѿ�������ӵ�
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
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;	// ��ȡ������

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
		// �������װ������������ô���ǾͲ�Ӧ�ü��������ƶ�
		// ��ͨ���Ƿ�װ������������ͻ��˶����ƶ��������⣬������Ӷ����RPC�����Ŀ�����
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
	// ��ȡ�ӿڴ�С
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ���׼��λ��
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// ����Ļ����ת��Ϊ��������
	FVector CrosshairWorldPosition;		// ׼��������������
	FVector CrosshairWorldDirection;	// ׼�����緽������
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	// �ж������Ƿ�ת���ɹ�
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

		// ���û�и��ٵ��κν����������׼��գ���׼����Ʒ����80000��)
		// ��������£����Ǿͽ�����������ΪEnd����
		if (!TraceHitResult.bBlockingHit)
		{
			// ���Ŀ��û������
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
			// ����ʮ��׼�ĵ���չֵ

			// �ٶ�ӳ��[0, 600] -> [0. 1]
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
