#pragma once

// ��������ö����

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),		// ���ǹ
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),	// �񵯷�����
	EWT_Pistol UMETA(DisplayName = "Pistol"),					// ��ǹ

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
