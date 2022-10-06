// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSION_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:

	virtual bool Initialize() override;
	// �ڹؿ��������ϱ��Ƴ�ʱ�ͻᱻ���ã������йؿ����������л�
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	/// <summary>
	/// Ϊ���˻Ự��ϵͳ��MultiPlayerSessionSubsystem������Զ���ί���ṩ�� �ص�
	/// </summary>
	UFUNCTION()
		void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
		void OnStartSession(bool bWasSuccessful);

private:

	// ָ����С������������֮�����ǵ���ͼ�ϵİ�ť�����������ӵ�c++�еİ�ť����
	// ע�⣬����	C++����������������ͼ��Ĳ���������ȫ��ͬ
	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		UButton* JoinButton;

	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	void MenuTearDown();

	// �����������߻Ự���ܵ���ϵͳ
	class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	int32 NumPublicConnections{ 4 };						// �Ự�������ӵ����������Ĭ��4
	FString MatchType{ TEXT("FreeForAll") };				// ƥ�����ͣ�Ĭ�� FreeForAll
	FString PathToLobby{ TEXT("") };						// ����·��
};
