// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionSubsystem.generated.h"

///
/// ���������Լ����Զ���ί�У����ڽ��ص��󶨵������ࣨ���˵��ش�ί�У�
///	���Դ���һ����̬�ಥί�У��Ӵ���㲥һ��
///
/// �������ί����һ����̬�ಥί�С�
///	 MULTICAST �ಥ����˼���ǣ�һ�������㲥���������Խ����ǵĻص������󶨵����ί����
///  DYNAMIC ��̬����˼���ǣ���ζ�ſ������л�ί�У����ҿ��Դ�һ����ͼ�ڲ�������߼�����Щί��
///
///	 һ�㱻�ƺ�Ϊ �¼����ȳ���
///
/// ����һ���ܹ���һ�������Ķಥ����
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
	/// ����Ự���ܣ��˵������е��ã����ԴӰ�������ϵͳ���κ����е�������
	/// </summary>

	// �����Ự���������ûỰ��ֵ��
	void CreateSession(int32 NumPublicConnections, FString MatchType);

	/**
	 * @brief ���һỰ
	 * @param MaxSearchResults ָ��Ҫ�����Ľ������
	 */
	void FindSessions(int32 MaxSearchResults);

	// ����Ҫ����ĻỰ�������������������Ϸ�Ự
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	// ���ٻỰ
	void DestroySession();

	// ��ʼ�Ự
	void StartSession();

	/// <summary>
	/// ��Ҫ�������Լ��Ĳ˵�����Զ���ί�����󶨻ص�
	/// </summary>
	///	����һ���µ�ί������
	FMultiPlayerOnCreateSessionComplete MultiPlayerOnCreateSessionComplete;
	FMultiPlayerOnFindSessionsComplete MultiPlayerOnFindSessionsComplete;
	FMultiPlayerOnJoinSessionComplete MultiPlayerOnJoinSessionComplete;
	FMultiPlayerOnDestroySessionComplete MultiPlayerOnDestroySessionComplete;
	FMultiPlayerOnStartSessionComplete MultiPlayerOnStartSessionComplete;

protected:
	// ���ᱻ��ӵ����߻Ự�ӿ�ί���б�����ڲ��ص�����
	// ����Ҫ�����������е���

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;		// ���߻Ự������ָ��
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;  // ��һ�λỰ����
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;		// ��һ�λỰ����

	// ��ӵ����߻Ự����ί���б���
	// ���� MultiPlayerSessionSubsystem �ڲ��Ļص����� �������Щί�н��а�

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;			// �����Ự���ί��
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;			// ���һỰ���ί��
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;				// ����Ự���ί��
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;		// ���ٻỰ���ί��
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;			// ��ʼ�Ự���ί��
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };		// ���Ự������ʱ�����ǽ������������������true�����ǽ�����һ���µĻỰ
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
