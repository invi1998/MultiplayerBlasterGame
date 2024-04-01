// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterHUD.h"

#include "CharacterOverlay.h"
#include "GameFramework/PlayerController.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/KismetSystemLibrary.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	OwnerPlayerController = OwnerPlayerController ? OwnerPlayerController : GetOwningPlayerController();
	if (OwnerPlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(OwnerPlayerController, CharacterOverlayClass);
		if (CharacterOverlay)
		{
			CharacterOverlay->AddToViewport();
		}
	}
}

void ABlasterHUD::AddAnnouncement()
{
	OwnerPlayerController = OwnerPlayerController ? OwnerPlayerController : GetOwningPlayerController();
	if (OwnerPlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(OwnerPlayerController, AnnouncementClass);
		if (Announcement)
		{
			Announcement->AddToViewport();
		}
	}
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwnerPlayerController = OwnerPlayerController ? OwnerPlayerController : GetOwningPlayerController();
	if (OwnerPlayerController && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwnerPlayerController, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			ElimMassages.Add(ElimAnnouncementWidget);	// ������Ϣ��ӵ������У��Ա���һ��ʱ����Ƴ�

			for (auto ElimMsg : ElimMassages)
			{
				if (ElimMsg && ElimMsg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ElimMsg->AnnouncementBox);	// ��ȡ������ۣ���������λ��

					if (CanvasSlot)
					{
						FVector2D ElimMsgPosition = CanvasSlot->GetPosition();	// ��Ϣ��λ��
						FVector2D NewPosition = FVector2D(ElimMsgPosition.X, ElimMsgPosition.Y + (ElimMassages.Num() - 1) * 50.f);	// �µ�λ��
						CanvasSlot->SetPosition(NewPosition);	// �����µ�λ��
						UKismetSystemLibrary::PrintString(this, "ElimMsgPosition: " + ElimMsgPosition.ToString() + " NewPosition: " + NewPosition.ToString(), true, true, FLinearColor::Red, 5.f);	// ��ӡ��Ϣλ��
					}
					
				}
			}

			// Ϊ����һ��ʱ����Ƴ�����Ϣ��������Ҫһ����ʱ��
			FTimerHandle ElimAnnouncementTimer;
			FTimerDelegate ElimAnnouncementTimerDelegate;	// �������ڰ󶨺���������ʱ������ʱ����
			ElimAnnouncementTimerDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);		// �󶨺���, ���ݵĲ�����ElimAnnouncement
			GetWorldTimerManager().SetTimer(ElimAnnouncementTimer, ElimAnnouncementTimerDelegate, ElimAnnouncementTime, false);	// ���ü�ʱ��
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, "ElimAnnouncementWidget is nullptr", true, true, FLinearColor::Red, 5.f);
		}
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();	// �Ӹ����Ƴ�
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	// ���������Ϸ��ʮ��׼�ģ�ʮ��׼��Ӧ������������
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		const FVector2D ViewPortCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreaScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewPortCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreaScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewPortCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreaScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewPortCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreaScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewPortCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreaScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewPortCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	// ���Ƶ����ģ���Ҫ����Ļ���ģ������������ƶ�TextureWidth/2, ����߶�/2
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		Color
	);
}
