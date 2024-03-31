// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"

#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();	// 将按钮添加到视口
	SetVisibility(ESlateVisibility::Visible);	// 设置按钮可见
	bIsFocusable = true;	// 设置按钮可聚焦

	if (const UWorld* World = GetWorld())
	{
		PlayerController = PlayerController ? PlayerController : Cast<APlayerController>(World->GetFirstPlayerController());		// 获取玩家控制器
		if (PlayerController)
		{
			FInputModeGameAndUI InputMode;	// 游戏和UI模式, 显示UI
			InputMode.SetWidgetToFocus(TakeWidget());	// 设置聚焦的控件
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);	// 设置鼠标锁定模式
			PlayerController->SetInputMode(InputMode);	// 设置输入模式
			PlayerController->SetShowMouseCursor(true);	// 显示鼠标
		}
	}

	if (ReturnButton && !ReturnButton->OnClicked.IsBound())		// 如果返回按钮存在并且未绑定点击事件
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);	// 绑定返回按钮点击事件
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();	// 获取多人游戏会话子系统
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);	// 绑定销毁会话完成事件
		}
	}

}

void UReturnToMainMenu::MenuTeardown()
{
	SetVisibility(ESlateVisibility::Hidden);	// 设置按钮不可见
	RemoveFromParent();	// 从父控件移除

	if (const UWorld* World = GetWorld())
	{
		PlayerController = PlayerController ? PlayerController : Cast<APlayerController>(World->GetFirstPlayerController());		// 获取玩家控制器
		if (PlayerController)
		{
			FInputModeGameOnly InputMode;	// 游戏模式, 不显示UI
			PlayerController->SetInputMode(InputMode);	// 设置输入模式
			PlayerController->SetShowMouseCursor(false);	// 隐藏鼠标
		}
	}

	if (ReturnButton && ReturnButton->OnClicked.IsBound())		// 如果返回按钮存在并且绑定点击事件
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);	// 取消绑定返回按钮点击事件
	}

	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())	// 如果多人游戏会话子系统存在并且绑定销毁会话完成事件
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);	// 取消绑定销毁会话完成事件
	}
}

bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);	// 启用返回按钮
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>())
		{
			GameMode->ReturnToMainMenuHost();	// 返回主菜单
		}
		else
		{
			// 如果没有找到游戏模式，我们将尝试使用玩家控制器
			PlayerController = PlayerController ? PlayerController : Cast<APlayerController>(World->GetFirstPlayerController());		// 获取玩家控制器
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());	// 返回主菜单
			}
		}
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);	// 禁用返回按钮

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();	// 销毁会话
	}
}
