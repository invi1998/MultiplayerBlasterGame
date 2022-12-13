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

// �������߳���
#define TRACE_LENGTH 80000.f

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

	// ע��װ��������
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
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
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		// �������װ������������ô���ǾͲ�Ӧ�ü��������ƶ�
		// ��ͨ���Ƿ�װ������������ͻ��˶����ƶ��������⣬������Ӷ����RPC�����Ŀ�����
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	// ս�������Ҫ֪�����º�̧�𿪻�ť��ʱ��
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		ServerFire();
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
		const FVector Start = CrosshairWorldPosition;

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// ���û�и��ٵ��κν����������׼��գ���׼����Ʒ����80000��)
		// ��������£����Ǿͽ�����������ΪEnd����
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			// ���Ŀ��û������
			HitTarget = End;
		}
		else
		{
			// ʹ�����Թ켣���Ƶ�������
			HitTarget = TraceHitResult.ImpactPoint;
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red
			);
		}
	}
}

void UCombatComponent::ServerFire_Implementation()
{
	MuticastFire();
}

void UCombatComponent::MuticastFire_Implementation()
{
	// ĿǰΪֹ���������Ҫ���ľ��ǲ��ſ�����̫�涯���Ͳ��ſ�����Ч
	if (EquippedWeapon == nullptr) return;

	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);

	// һ��bOrientRotationToMovement����ΪTrue�󣬽�ɫ�ĳ����ת���ƶ��ķ���
	// һ��bOrientRotationToMovement ΪFalse������bUseControllerDesiredRotation����ΪTrue��
	// ��ô�ڽ�ɫ�ƶ��У����Controller�ĳ���ͽ�ɫ����һ�£�
	// ��ɫ����ͻ�ƽ���İ���RotationRate�����õĸ��������ת��������ת��ɫ��Rotation��Controller�ĳ���
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}