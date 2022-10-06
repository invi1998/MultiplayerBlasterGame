// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionSubsystem.generated.h"

///
/// 声明我们自己的自定义委托，用于将回调绑定到带单类（给菜单回传委托）
///	可以创建一个动态多播委托，从此类广播一次
///
/// 下面这个委托是一个动态多播委托。
///	 MULTICAST 多播的意思就是：一旦它被广播，多个类可以将他们的回调函数绑定到这个委托上
///  DYNAMIC 动态的意思就是：意味着可以序列化委托，并且可以从一个蓝图内部保存或者加载这些委托
///
///	 一般被称呼为 事件调度程序
///
/// 声明一个能够绑定一个参数的多播函数
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiPlayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();

	/// <summary>
	/// 处理会话功能，菜单类会进行调用，可以从包含此子系统的任何类中调用他们
	/// </summary>

	// 创建会话并负责设置会话键值对
	void CreateSession(int32 NumPublicConnections, FString MatchType);

	/**
	 * @brief 查找会话
	 * @param MaxSearchResults 指定要搜索的结果数量
	 */
	void FindSessions(int32 MaxSearchResults);

	// 传入要加入的会话搜索结果，进而加入游戏会话
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	// 销毁会话
	void DestroySession();

	// 开始会话
	void StartSession();

	/// <summary>
	/// 需要用我们自己的菜单类的自定义委托来绑定回调
	/// </summary>
	///	声明一个新的委托类型
	FMultiPlayerOnCreateSessionComplete MultiPlayerOnCreateSessionComplete;
	FMultiPlayerOnFindSessionsComplete MultiPlayerOnFindSessionsComplete;
	FMultiPlayerOnJoinSessionComplete MultiPlayerOnJoinSessionComplete;
	FMultiPlayerOnDestroySessionComplete MultiPlayerOnDestroySessionComplete;
	FMultiPlayerOnStartSessionComplete MultiPlayerOnStartSessionComplete;

protected:
	// 将会被添加到在线会话接口委托列表里的内部回调函数
	// 不需要在这个类外进行调用

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;		// 在线会话的智能指针
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;  // 上一次会话设置
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;		// 上一次会话搜索

	// 添加到在线会话界面委托列表中
	// 我们 MultiPlayerSessionSubsystem 内部的回调函数 将会对这些委托进行绑定

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;			// 创建会话完成委托
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;			// 查找会话完成委托
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;				// 加入会话完成委托
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;		// 销毁会话完成委托
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;			// 开始会话完成委托
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };		// 当会话被销毁时，我们将检测这个变量，如果是true，我们将创建一个新的会话
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
