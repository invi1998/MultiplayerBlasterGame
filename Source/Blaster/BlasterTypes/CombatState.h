#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),		// δռ��
	ECS_Reloading UMETA(DisplayName = "Reloading"),		// ����װ��
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),	// ����
	ECS_SwappingWeapon UMETA(DisplayName = "Swapping Weapon"),		// �л�����

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};
