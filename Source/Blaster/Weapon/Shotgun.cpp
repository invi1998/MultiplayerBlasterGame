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

		// 计算霰弹枪子弹命中个数

		// 因为霰弹枪子弹散射的缘故，它有可能同时命中多个角色，我们也需要处理这种情况
		// 这里采用一个map结构处理，键为角色指针，值为子弹命中数量
		// 击中角色和对应数量的map映射
		TMap<ABlasterCharacter*, uint32> HitMap;			// 用于存储击中角色和对应数量的map映射
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;	// 用于存储击中角色和对应数量的map映射 (头部击中)

		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
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

		// 计算伤害并应用在角色身上
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

		// 计算爆头伤害并应用在角色身上
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

		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();	// 是否造成伤害，如果不使用服务器倒带，或者是本地控制，那么就造成伤害

		for (auto DamagePair : HitDamageMap)
		{
			if (DamagePair.Key && HasAuthority() && bCauseAuthDamage && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					DamagePair.Key,			// 被击中的角色
					DamagePair.Value,		// 伤害
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
	// 从武器的枪口位置开始追踪
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// 一个从跟踪起始位置指向命中目标的向量（归一化的向量）
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// 从轨迹起点到头部目标的某个位置
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
