// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharater = Cast<ACharacter>(GetOwner());
	if (OwnerCharater)
	{
		AController* OwnerController = OwnerCharater->Controller;
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}