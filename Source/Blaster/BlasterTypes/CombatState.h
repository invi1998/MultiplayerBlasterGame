#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),		// δռ��
	ECS_Reloading UMETA(DisplayName = "Reloading"),		// ����װ��
	ECS_ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"),	// ����

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};
