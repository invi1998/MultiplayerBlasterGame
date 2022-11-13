// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
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

	bIsCrouched = BlasterCharacter->bIsCrouched;

	bAiming = BlasterCharacter->IsAiming();

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
}