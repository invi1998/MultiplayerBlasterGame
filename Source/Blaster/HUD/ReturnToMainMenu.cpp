// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"

#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();	// ����ť��ӵ��ӿ�
	SetVisibility(ESlateVisibility::Visible);	// ���ð�ť�ɼ�
	bIsFocusable = true;	// ���ð�ť�ɾ۽�

	if (const UWorld* World = GetWorld())
	{
		PlayerController = PlayerController ? PlayerController : Cast<APlayerController>(World->GetFirstPlayerController());		// ��ȡ��ҿ�����
		if (PlayerController)
		{
			FInputModeGameAndUI InputMode;	// ��Ϸ��UIģʽ, ��ʾUI
			InputMode.SetWidgetToFocus(TakeWidget());	// ���þ۽��Ŀؼ�
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);	// �����������ģʽ
			PlayerController->SetInputMode(InputMode);	// ��������ģʽ
			PlayerController->SetShowMouseCursor(true);	// ��ʾ���
		}
	}

	if (ReturnButton)
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);	// �󶨷��ذ�ť����¼�
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();	// ��ȡ������Ϸ�Ự��ϵͳ
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);	// �����ٻỰ����¼�
		}
	}

}

void UReturnToMainMenu::MenuTeardown()
{
	SetVisibility(ESlateVisibility::Hidden);	// ���ð�ť���ɼ�
	RemoveFromParent();	// �Ӹ��ؼ��Ƴ�

	if (const UWorld* World = GetWorld())
	{
		PlayerController = PlayerController ? PlayerController : Cast<APlayerController>(World->GetFirstPlayerController());		// ��ȡ��ҿ�����
		if (PlayerController)
		{
			FInputModeGameOnly InputMode;	// ��Ϸģʽ, ����ʾUI
			PlayerController->SetInputMode(InputMode);	// ��������ģʽ
			PlayerController->SetShowMouseCursor(false);	// �������
		}
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
		ReturnButton->SetIsEnabled(true);	// ���÷��ذ�ť
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>())
		{
			GameMode->ReturnToMainMenuHost();	// �������˵�
		}
		else
		{
			// ���û���ҵ���Ϸģʽ�����ǽ�����ʹ����ҿ�����
			PlayerController = PlayerController ? PlayerController : Cast<APlayerController>(World->GetFirstPlayerController());		// ��ȡ��ҿ�����
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());	// �������˵�
			}
		}
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);	// ���÷��ذ�ť

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();	// ���ٻỰ
	}
}
