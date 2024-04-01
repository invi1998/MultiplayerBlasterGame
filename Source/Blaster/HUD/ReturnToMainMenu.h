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

	void MenuSetup();		// ����ť��ӵ��ӿ�

	void MenuTeardown();	// ����ť���ӿ��Ƴ�

protected:
	virtual bool Initialize() override;	// ��ʼ��

	UFUNCTION()	// �����Ự���
	void OnDestroySession(bool bWasSuccessful);	// ���ٻỰ���

	UFUNCTION()	// ����뿪��Ϸ
	void OnPlayerLeftGame();	// ����뿪��Ϸ

private:
	UPROPERTY(meta = (BindWidget))	// �󶨰�ť
	class UButton* ReturnButton;	// �������˵���ť

	UFUNCTION()	// ���ذ�ť����¼�
	void ReturnButtonClicked();	// ���ذ�ť����¼�

	UPROPERTY()	// ������Ϸ�Ự��ϵͳ
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;	// ������Ϸ�Ự��ϵͳ

	UPROPERTY()	// ��ҿ�����
	class APlayerController* PlayerController;	// ��ҿ�����
	
};
