// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
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

	if (EquippedWeapon == nullptr) return;

	if (Character && bFireButtonPressed)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire();
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
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ע��װ��������
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
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