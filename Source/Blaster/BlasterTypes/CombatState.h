#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),		// Œ¥’º”√
	ECS_Reloading UMETA(DisplayName = "Reloading"),		// Œ‰∆˜◊∞ÃÓ
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),	// »”¡ÒµØ
	ECS_SwappingWeapon UMETA(DisplayName = "Swapping Weapon"),		// «–ªªŒ‰∆˜

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};
