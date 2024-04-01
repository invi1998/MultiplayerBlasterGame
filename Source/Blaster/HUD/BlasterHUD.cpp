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

			ElimMassages.Add(ElimAnnouncementWidget);	// 将该消息添加到数组中，以便在一段时间后移除

			for (auto ElimMsg : ElimMassages)
			{
				if (ElimMsg && ElimMsg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ElimMsg->AnnouncementBox);	// 获取画布插槽，用于设置位置

					if (CanvasSlot)
					{
						FVector2D ElimMsgPosition = CanvasSlot->GetPosition();	// 消息的位置
						FVector2D NewPosition = FVector2D(ElimMsgPosition.X, ElimMsgPosition.Y + (ElimMassages.Num() - 1) * 50.f);	// 新的位置
						CanvasSlot->SetPosition(NewPosition);	// 设置新的位置
						UKismetSystemLibrary::PrintString(this, "ElimMsgPosition: " + ElimMsgPosition.ToString() + " NewPosition: " + NewPosition.ToString(), true, true, FLinearColor::Red, 5.f);	// 打印消息位置
					}
					
				}
			}

			// 为了在一段时间后移除该消息，我们需要一个计时器
			FTimerHandle ElimAnnouncementTimer;
			FTimerDelegate ElimAnnouncementTimerDelegate;	// 代理，用于绑定函数，当计时器结束时调用
			ElimAnnouncementTimerDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);		// 绑定函数, 传递的参数是ElimAnnouncement
			GetWorldTimerManager().SetTimer(ElimAnnouncementTimer, ElimAnnouncementTimerDelegate, ElimAnnouncementTime, false);	// 设置计时器
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
		MsgToRemove->RemoveFromParent();	// 从父级移除
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	// 绘制射击游戏的十字准心，十字准心应该在武器类上
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
	// 绘制点中心，需要在屏幕中心，按纹理向左移动TextureWidth/2, 纹理高度/2
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
