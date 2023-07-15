#pragma once


// 定义走线长度 (武器子弹的最长发射距离）
#define TRACE_LENGTH 80000.f

// 武器类型枚举类

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),		// 突击步枪
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),	// 榴弹发射器
	EWT_Pistol UMETA(DisplayName = "Pistol"),					// 手枪
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),	// 冲锋枪
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),					// 霰弹枪

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
