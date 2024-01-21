// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;	// ��������

	PickupEffectNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectNiagaraComponent"));		// ����ʰȡ��Ч
	PickupEffectNiagaraComponent->SetupAttachment(RootComponent);		// ����ʰȡ��Ч�ĸ����Ϊ�����



}

void AHealthPickup::Destroyed()
{
	if (PickupEffectNiagaraSystem)
	{
		// ��ϣ��Ѫ����ʰȡ����һ�����ٵ���Ч�����������ﴴ����һ��NiagaraSystem������NiagaraComponent�Ĵ�����ʽ��ͬ��
		// ��������ʹ����NiagaraFunctionLibrary::SpawnSystemAtLocation()����������NiagaraSystem
		// ���������Ҫ����һ��WorldContextObject���������Ǵ���this����Ϊ����ϣ�������Actor��λ�ô�����Ч
		// Ȼ����NiagaraSystem���������Ǵ�����PickupEffectNiagaraSystem�����������HealthPickup.h�ж���
		// �����λ�ú���ת���������Ǵ�����Ѫ����λ�ú���ת
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffectNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	
	// �����ø����Destroyed()����
	Super::Destroyed();
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
	}
	Destroy();
}
