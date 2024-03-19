#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),		// Œ¥’º”√
	ECS_Reloading UMETA(DisplayName = "Reloading"),		// Œ‰∆˜◊∞ÃÓ
	ECS_ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"),	// »”¡ÒµØ

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};
