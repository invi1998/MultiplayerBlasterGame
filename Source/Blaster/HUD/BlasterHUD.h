// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

// ĿǰΪֹ��BlasterHUD����֪��Ҫ������Щ������Ļ����Ϊʮ��׼��
// ���������ṩһ���ṹ�壬����Ӧ���еĲ�ͬʮ��׼�ĵ�����
USTRUCT(BlueprintType)
struct FHUDPackage
{
	// ����������ɺ� �Ա���õķ���ϵͳ���Խ����ǵ�FHUDPackageҲ���뵽������
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairsCenter;
	class UTexture2D* CrosshairsLeft;
	class UTexture2D* CrosshairsRight;
	class UTexture2D* CrosshairsTop;
	class UTexture2D* CrosshairsBottom;
	float CrosshairSpread;		// ��ֵ���ڿ���ʮ��׼��Ӧ�÷�ɢ����
	FLinearColor CrosshairsColor;	// ʮ��׼�ĵ���ɫ
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
		TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcement")
		TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
		class UAnnouncement* Announcement;

	void AddCharacterOverlay();
	void AddAnnouncement();

protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

	UPROPERTY(EditAnywhere)
		float CrosshairSpreadMax = 16.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
