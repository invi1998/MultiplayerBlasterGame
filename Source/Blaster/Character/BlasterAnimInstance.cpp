// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
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

	// ��ȡ��ɫ�ٶ�ʸ��
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Spead = Velocity.Size();

	// ͨ����ȡ��ɫ�Ƿ��ڵ���״̬�� ��ֵ bIsInAir
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	// �ж� ��ȡ��ǰ���ٶ� �Ƿ����0���ж��Ƿ��ڼ���״̬
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	// �жϵ�ǰ��ɫ�Ƿ�װ������������״̬��ֵ�� bWeaponEquipped
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;

	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();

	// offset Yaw for Strafing (��ȡɨ�䶯����ƫ��Yaw)
	// Ϊ�˵õ���б����Ϣ��������Ҫ֪�����������ƶ��ķ����Լ������ƶ�������׼�Ŀ�����ָ��ķ���
	// �� UE �У�ƫ���� Yaw ��ָ����ɫ Z �ᣨҲ���Ƿ����ϵ��Ǹ��ᣩ��ת�Ľǡ�
	// ƫ���� Yaw ʹ�������� Z ����ת��ƫ������ת��Ч�����������������ԭ��ת����
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//if (!BlasterCharacter->HasAuthority() && !BlasterCharacter->IsLocallyControlled())
	//{
	//	// ��ӡ��־
	//	UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f : "), AimRotation.Yaw);
	//	UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f : "), MovementRotation.Yaw);
	//}
	// ��ȡƫ��
	// Ϊ���ǵ�������ת��ȡһ���洢������Ȼ��ƽ���Ĳ������ת
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	// ɨ�䶯������б��
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.0f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	// DeltaTime ʵ���Ǿ�����һ֡���ù�ȥ�˶��,���֡�����ȶ���,��ô����ֵΪ 1/fps ,��fps=100ʱ,��DeltaTime=1/100=0.01
	// ʵ������֡��һ���ǲ��ȶ���, �������ÿ����Ҫ�ı仯��, ����DeltaTime��ƽ���֡�ʵ�Ӱ��
	// ��UE5�У���������Ϊ DeltaSeconds

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// ������Ҫ������װ���������ϵĲ���л�ȡ���ת��
		// ��������۵�����ռ�任��ֵ�����ǵĳ�Ա���� LeftHandTransform
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		// �����������������У���������ʵ��Ҫת���������ǹ����ϵ�����
		FVector OutPosition;
		FRotator OutRotation;
		// TransformFromBoneSpace���������Ҫ�����������
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		// �ڵ�������������󣬱任�������ռ䣬�����������ת������洢������������������ϵ���ȷλ�ú���ת���ݣ�
		// Ȼ���������������ø����ǵ����ֱ任
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		// �Դˣ����ǾͿ�������ͼ��ʹ����� LeftHandTransform

		// �޸�����ָ���ʮ��׼�Ĳ�һ�µ�����
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookatRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookatRotation, DeltaSeconds, 30.f);
		}

		/*��������
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Green);
		*/
	}
}