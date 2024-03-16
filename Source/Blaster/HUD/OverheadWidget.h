// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);

	// 显示pown的网络角色
	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* InPawn);

protected:
	// UE5.1之后该方法被弃用，使用下面的方法代替
	// virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
	virtual void NativeDestruct() override;	// 该方法在UMG部件被销毁时调用
};
