// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharacter == nullptr)
	{
		return;
	}

	// 获取角色速度矢量
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Spead = Velocity.Size();

	// 通过获取角色是否处于掉落状态来 赋值 bIsInAir
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	// 判断 获取当前加速度 是否大于0来判断是否处于加速状态
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	// 判断当前角色是否装备了武器，将状态赋值给 bWeaponEquipped
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;

	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();

	// offset Yaw for Strafing (获取扫射动作的偏移Yaw)
	// 为了得到倾斜角信息，我们需要知道我们正在移动的方向以及我们移动方向瞄准的控制器指向的方向
	// 在 UE 中，偏航角 Yaw 是指绕蓝色 Z 轴（也就是方向朝上的那个轴）旋转的角。
	// 偏航角 Yaw 使人物绕着 Z 轴旋转。偏航角旋转的效果，可以想象成人在原地转动。
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//if (!BlasterCharacter->HasAuthority() && !BlasterCharacter->IsLocallyControlled())
	//{
	//	// 打印日志
	//	UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f : "), AimRotation.Yaw);
	//	UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f : "), MovementRotation.Yaw);
	//}
	// 获取偏移
	// 为我们的增量旋转获取一个存储变量，然后平滑的插入该旋转
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	// 扫射动作的倾斜角
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.0f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	// DeltaTime 实际是距离上一帧调用过去了多久,如果帧率是稳定的,那么此数值为 1/fps ,例fps=100时,则DeltaTime=1/100=0.01
	// 实际运行帧率一定是不稳定的, 则可以用每秒需要的变化量, 乘以DeltaTime来平衡掉帧率的影响
	// 在UE5中，它被更名为 DeltaSeconds

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// 我们需要从我们装备的武器上的插槽中获取插槽转换
		// 将武器插槽的世界空间变换赋值给我们的成员变量 LeftHandTransform
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		// 现在是在世界坐标中，但是我们实际要转换的是我们骨骼上的坐标
		FVector OutPosition;
		FRotator OutRotation;
		// TransformFromBoneSpace这个函数需要传入骨骼名称
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		// 在调用完这个函数后，变换到骨骼空间，这个向量和旋转器将会存储左手握在武器插槽在上的正确位置和旋转数据，
		// 然后将这两个数据设置给我们的左手变换
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		// 自此，我们就可以在蓝图中使用这个 LeftHandTransform

		// 修复武器指向和十字准心不一致的问题
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookatRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookatRotation, DeltaSeconds, 30.f);
		}

		/*调试线条
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Green);
		*/
	}

	bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}