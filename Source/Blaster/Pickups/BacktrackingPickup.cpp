// Fill out your copyright notice in the Description page of Project Settings.


#include "BacktrackingPickup.h"

#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

ABacktrackingPickup::ABacktrackingPickup()
{
}

void ABacktrackingPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* Buff = BlasterCharacter->GetBuff())
		{
			Buff->AddBacktracking(BacktrackingTime, BuffAliveTime, BacktrackingCostTime);
		}
	}

}
