// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	UWorld* World = GetWorld();

	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 从枪口插槽到十字准心线轨迹的命中位置
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotator = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority())		// 服务端
			{
				if (InstigatorPawn->IsLocallyControlled())	// 服务端，本地控制，即服务端玩家，生成属性复制的子弹，不倒带
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotator,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
				else	// 服务端，非本地控制，即客户端玩家，生成非属性复制的子弹，倒带
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ServerSideRewindProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotator,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			else	// 客户端，使用服务端倒带
			{
				if (InstigatorPawn->IsLocallyControlled())	// 客户端，本地控制，即客户端本机玩家，生成非属性复制的子弹，倒带
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ServerSideRewindProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotator,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = true;		// 设置倒带标志
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();	// 设置倒带位置
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;	// 设置倒带初始速度（精度为0.01）
					SpawnedProjectile->Damage = Damage;	// 设置伤害
				}
				else	// 客户端，非本地控制，即其他客户端机器玩家，生成非属性复制的子弹，不倒带，他会在自己的客户端进行倒带
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotator,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else	// 不使用服务端倒带
		{
			if (InstigatorPawn->HasAuthority())	// 服务端，所有的子弹都是属性复制的
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotator,
					SpawnParams
				);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
		}
		
	}
}