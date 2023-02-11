#pragma once

// 武器类型枚举类

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
