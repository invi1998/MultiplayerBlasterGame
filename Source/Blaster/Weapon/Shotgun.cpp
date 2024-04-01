// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		// ��������ǹ�ӵ����и���

		// ��Ϊ����ǹ�ӵ�ɢ���Ե�ʣ����п���ͬʱ���ж����ɫ������Ҳ��Ҫ�����������
		// �������һ��map�ṹ������Ϊ��ɫָ�룬ֵΪ�ӵ���������
		// ���н�ɫ�Ͷ�Ӧ������mapӳ��
		TMap<ABlasterCharacter*, uint32> HitMap;			// ���ڴ洢���н�ɫ�Ͷ�Ӧ������mapӳ��
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;	// ���ڴ洢���н�ɫ�Ͷ�Ӧ������mapӳ�� (ͷ������)

		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				const bool bIsHeadShot = FireHit.BoneName.ToString() == "head";

				if (bIsHeadShot)
				{
					if (HeadShotHitMap.Contains(BlasterCharacter))
					{
						HeadShotHitMap[BlasterCharacter]++;
					}
					else
					{
						HeadShotHitMap.Emplace(BlasterCharacter, 1);
					}
				}
				else
				{
					if (HitMap.Contains(BlasterCharacter))
					{
						HitMap[BlasterCharacter]++;
					}
					else
					{
						HitMap.Emplace(BlasterCharacter, 1);
					}
				}
				
			}


			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					0.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}

		}

		TArray<ABlasterCharacter*> HitCharacters;
		TMap<ABlasterCharacter*, float> HitDamageMap;

		// �����˺���Ӧ���ڽ�ɫ����
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				if (HitDamageMap.Contains(HitPair.Key))
				{
					HitDamageMap[HitPair.Key] += Damage * HitPair.Value;
				}
				else
				{
					HitDamageMap.Emplace(HitPair.Key, Damage * HitPair.Value);
				}
				
				HitCharacters.AddUnique(HitPair.Key);
			}
		}

		// ���㱬ͷ�˺���Ӧ���ڽ�ɫ����
		for (auto HitHeadPair : HeadShotHitMap)
		{
			if (HitHeadPair.Key)
			{
				if (HitDamageMap.Contains(HitHeadPair.Key))
				{
					HitDamageMap[HitHeadPair.Key] += HeadShotDamage * HitHeadPair.Value;
				}
				else
				{
					HitDamageMap.Emplace(HitHeadPair.Key, HeadShotDamage * HitHeadPair.Value);
				}

				HitCharacters.AddUnique(HitHeadPair.Key);
			}
		}

		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();	// �Ƿ�����˺��������ʹ�÷����������������Ǳ��ؿ��ƣ���ô������˺�

		for (auto DamagePair : HitDamageMap)
		{
			if (DamagePair.Key && HasAuthority() && bCauseAuthDamage && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					DamagePair.Key,			// �����еĽ�ɫ
					DamagePair.Value,		// �˺�
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}

		if (!HasAuthority() && bUseServerSideRewind)
		{
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;

			if (BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest_Shotgun(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime,
					this
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceHitWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	// ��������ǹ��λ�ÿ�ʼ׷��
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// һ���Ӹ�����ʼλ��ָ������Ŀ�����������һ����������
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// �ӹ켣��㵽ͷ��Ŀ���ĳ��λ��
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;

		/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Magenta, true);
		DrawDebugSphere(GetWorld(), EndLoc, 4.0f, 12, FColor::Orange, true);
		DrawDebugLine(
			GetWorld(),
			TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
			FColor::Green,
			true);*/

		HitTargets.Add(FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()));
	}
}
