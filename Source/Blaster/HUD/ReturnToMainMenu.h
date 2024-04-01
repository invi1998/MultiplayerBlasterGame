// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	void MenuSetup();		// 将按钮添加到视口

	void MenuTeardown();	// 将按钮从视口移除

protected:
	virtual bool Initialize() override;	// 初始化

	UFUNCTION()	// 创建会话完成
	void OnDestroySession(bool bWasSuccessful);	// 销毁会话完成

	UFUNCTION()	// 玩家离开游戏
	void OnPlayerLeftGame();	// 玩家离开游戏

private:
	UPROPERTY(meta = (BindWidget))	// 绑定按钮
	class UButton* ReturnButton;	// 返回主菜单按钮

	UFUNCTION()	// 返回按钮点击事件
	void ReturnButtonClicked();	// 返回按钮点击事件

	UPROPERTY()	// 多人游戏会话子系统
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;	// 多人游戏会话子系统

	UPROPERTY()	// 玩家控制器
	class APlayerController* PlayerController;	// 玩家控制器
	
};
