#pragma once

// 武器类型枚举类

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),		// 冲锋枪
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),	// 榴弹发射器
	EWT_Pistol UMETA(DisplayName = "Pistol"),					// 手枪

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
