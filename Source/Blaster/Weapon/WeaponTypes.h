#pragma once


// �������߳��� (�����ӵ����������룩
#define TRACE_LENGTH 80000.f

// ��������ö����

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),		// ͻ����ǹ
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),	// ���Ͳ������
	EWT_Pistol UMETA(DisplayName = "Pistol"),					// ��ǹ
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),	// ���ǹ
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),					// ����ǹ
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),		// �ѻ���ǹ
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),		// �񵯷�����

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
