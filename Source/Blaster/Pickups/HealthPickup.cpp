// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;	// 开启复制

	PickupEffectNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectNiagaraComponent"));		// 创建拾取特效
	PickupEffectNiagaraComponent->SetupAttachment(RootComponent);		// 设置拾取特效的父组件为根组件



}

void AHealthPickup::Destroyed()
{
	if (PickupEffectNiagaraSystem)
	{
		// 我希望血宝被拾取后，有一个销毁的特效，所以我这里创建了一个NiagaraSystem，这与NiagaraComponent的创建方式不同，
		// 这里我们使用了NiagaraFunctionLibrary::SpawnSystemAtLocation()方法来创建NiagaraSystem
		// 这个方法需要传入一个WorldContextObject，这里我们传入this，因为我们希望在这个Actor的位置创建特效
		// 然后传入NiagaraSystem，这里我们传入了PickupEffectNiagaraSystem，这个变量在HealthPickup.h中定义
		// 最后传入位置和旋转，这里我们传入了血宝的位置和旋转
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffectNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
	
	// 最后调用父类的Destroyed()方法
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
