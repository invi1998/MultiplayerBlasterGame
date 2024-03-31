// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"

#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

AJumpPickup::AJumpPickup()
{
}

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		if (UBuffComponent* Buff = BlasterCharacter->GetBuff())
		{
			Buff->AddJump(JumpZVelocity, JumpBuffTime);
		}
	}
	Destroy();
}
