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
		// ��ǹ�ڲ�۵�ʮ��׼���߹켣������λ��
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotator = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority())		// �����
			{
				if (InstigatorPawn->IsLocallyControlled())	// ����ˣ����ؿ��ƣ����������ң��������Ը��Ƶ��ӵ���������
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
				else	// ����ˣ��Ǳ��ؿ��ƣ����ͻ�����ң����ɷ����Ը��Ƶ��ӵ�������
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
			else	// �ͻ��ˣ�ʹ�÷���˵���
			{
				if (InstigatorPawn->IsLocallyControlled())	// �ͻ��ˣ����ؿ��ƣ����ͻ��˱�����ң����ɷ����Ը��Ƶ��ӵ�������
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ServerSideRewindProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotator,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = true;		// ���õ�����־
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();	// ���õ���λ��
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;	// ���õ�����ʼ�ٶȣ�����Ϊ0.01��
					SpawnedProjectile->Damage = Damage;	// �����˺�
				}
				else	// �ͻ��ˣ��Ǳ��ؿ��ƣ��������ͻ��˻�����ң����ɷ����Ը��Ƶ��ӵ������������������Լ��Ŀͻ��˽��е���
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
		else	// ��ʹ�÷���˵���
		{
			if (InstigatorPawn->HasAuthority())	// ����ˣ����е��ӵ��������Ը��Ƶ�
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