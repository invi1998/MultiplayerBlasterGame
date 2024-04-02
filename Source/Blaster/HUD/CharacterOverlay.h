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
	class UProgressBar* HealthBar;	// 血条

	UPROPERTY(meta=(BindWidget))
	class UImage* HealthBarNative;	// 血条

	//UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	//class UMaterialInstanceDynamic* HealthBarMaterial;	// 血条材质

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthAdditionBar;	// 血条增加

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthSubtractionBar;	// 血条减少

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar_L2R;	// 护盾条 从左到右

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar_R2L;	// 护盾条 从右到左

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;	// 血量

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;		// 护盾

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;	// 分数

	UPROPERTY(meta=(BindWidget))
	UTextBlock* BlueTeamScore;	// 蓝队分数

	UPROPERTY(meta=(BindWidget))
	UTextBlock* RedTeamScore;	// 红队分数

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;	// 击杀数

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;	// 武器弹药

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;	// 携带弹药

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;	// 比赛倒计时

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;	// 榴弹

	UPROPERTY(meta=(BindWidget))
	UImage* HighPingImage;			// 高延迟图标

	UPROPERTY(meta = (BindWidgetAnim), Transient)	// 标记为瞬态，也就是说这个变量不会被保存到内存中
	class UWidgetAnimation* HighPingAnimation;	// 高延迟动画
};
