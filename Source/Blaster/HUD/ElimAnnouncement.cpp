// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	FString ElimText = AttackerName + " eliminated " + VictimName;
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimText));
	}
}
