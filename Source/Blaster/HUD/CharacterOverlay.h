// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
		class UProgressBar* HealthBar;	// Ѫ��

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthAdditionBar;	// Ѫ������

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthSubtractionBar;	// Ѫ������

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar_L2R;	// ������ ������

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar_R2L;	// ������ ���ҵ���

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* HealthText;	// Ѫ��

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;		// ����

	UPROPERTY(meta = (BindWidget))
		UTextBlock* ScoreAmount;	// ����

	UPROPERTY(meta = (BindWidget))
		UTextBlock* DefeatsAmount;	// ��ɱ��

	UPROPERTY(meta = (BindWidget))
		UTextBlock* WeaponAmmoAmount;	// ������ҩ

	UPROPERTY(meta = (BindWidget))
		UTextBlock* CarriedAmmoAmount;	// Я����ҩ

	UPROPERTY(meta = (BindWidget))
		UTextBlock* MatchCountdownText;	// ��������ʱ

	UPROPERTY(meta = (BindWidget))
		UTextBlock* GrenadesText;	// ��
};
