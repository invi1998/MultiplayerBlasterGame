// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerController.h"

#include "Animation/WidgetAnimation.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/Announcement.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/ReturnToMainMenu.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 绑定Esc键
	if (InputComponent)
	{
		InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
	}

}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();

	CheckTimeSync(DeltaSeconds);

	PollInit();

	CheckHighPing(DeltaSeconds);
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScore->SetVisibility(ESlateVisibility::Hidden);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterPlayerController::InitializeTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;

	if (bHUDValid)
	{
		const FString RedTeamScoreString = FString::Printf(TEXT("%d"), 0);
		const FString BlueTeamScoreString = FString::Printf(TEXT("%d"), 0);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(RedTeamScoreString));
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(BlueTeamScoreString));
		BlasterHUD->CharacterOverlay->RedTeamScore->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetVisibility(ESlateVisibility::Visible);
		bInitializeTeamScores = false;
	}
	else
	{
		bInitializeTeamScores = true;
	}
}

void ABlasterPlayerController::SetHUDRedTeamScores(int32 Scores)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), Scores);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
		BlasterHUD->CharacterOverlay->RedTeamScore->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Scores;
	}
}

void ABlasterPlayerController::SetHUDBlueTeamScores(int32 Scores)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), Scores);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Scores;
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

float ABlasterPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName state, bool bTeamMatch)
{
	MatchState = state;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
	
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	/*if (HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}*/

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
		
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;

			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) {
					SetHUDShield(HUDShield, HUDMaxShield);
					SetHUDShieldNative(HUDShield, HUDMaxShield);
				}
				/*SetHUDHealthAddition(HUDHealthAddition, HUDMaxHealth);
				SetHUDHealthSubtraction(HUDHealthSubtraction, HUDMaxHealth);*/
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeBeforeHealth) SetHUDHealthNative(HUDHealth, HUDMaxHealth, HUDHealth, HUDHealth);
				if (!HasAuthority() && bInitializeTeamScores && bShowTeamScores) InitializeTeamScores();

				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenadeCount());
				}
				
			}
		}
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;

	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamMatch;

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay == nullptr)
		{
			BlasterHUD->AddCharacterOverlay();
		}
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		if (!HasAuthority()) return;

		if (bTeamMatch)
		{
			InitializeTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		const bool bHUDValid = BlasterHUD->Announcement &&
			BlasterHUD->Announcement->AnnouncementText && 
			BlasterHUD->Announcement->InfoText;
		if (bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (BlasterGameState && BlasterPlayerState)
			{
				const TArray<ABlasterPlayerState*>TopPlayers = BlasterGameState->TopScoringPlayers;
				const FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombat())
	{
		BlasterCharacter->bDisableGamePlay = true;

		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}

}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0, 5);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ABlasterPlayerController::CheckHighPing(float DeltaSeconds)
{
	HighPingWarningTime += DeltaSeconds;
	if (HighPingWarningTime >= HighPingWarningFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : PlayerState;
		if (PlayerState)
		{
			// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Ping: %d"), static_cast<int>(PlayerState->GetPingInMilliseconds())));
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingWarningTime = 0.f;
	}
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingAnimation && BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaSeconds;
		if (PingAnimationRunningTime >= HighPingWarningDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitializeTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

FString ABlasterPlayerController::GetInfoText(const TArray<ABlasterPlayerState*>& TopPlayers)
{
	FString InfoTextString;

	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (!BlasterPlayerState) return InfoTextString;
	
	if (TopPlayers.Num() == 0)
	{
		InfoTextString = Announcement::NoWinner;
	}
	else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (TopPlayers.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("获胜者: \n%s"), *TopPlayers[0]->GetPlayerName());
	}
	else if (TopPlayers.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (const auto TiedPlayer : TopPlayers)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString ABlasterPlayerController::GetTeamsInfoText(ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::NoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;

	OnMatchStateSet(MatchState);

	// 只有在HUD组件构建成功，并且游戏状态处于等待开始的阶段，才进行公告UI展示
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();

		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

	}
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	// 通信往返时间
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f;
	// 计算服务器当前时间
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDHealthNative(float Health, float MaxHealth, float BeforeHealth, float AfterHealHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBarNative;

	if (bHUDValid)
	{
		const float BeforeHealthPercent = BeforeHealth / MaxHealth;
		const float HealthPercent = Health / MaxHealth;
		const float AfterHealHealthPercent = AfterHealHealth / MaxHealth;
		if (UMaterialInstanceDynamic* HealthBarMaterial = BlasterHUD->CharacterOverlay->HealthBarNative->GetDynamicMaterial())
		{
			HealthBarMaterial->SetScalarParameterValue(FName("Before"), BeforeHealthPercent);
			HealthBarMaterial->SetScalarParameterValue(FName("Current"), HealthPercent);
			HealthBarMaterial->SetScalarParameterValue(FName("After"), AfterHealHealthPercent);
			// 设置播放动画时间范围
		}
		
	}
	else
	{
		bInitializeBeforeHealth = true;
		HUDBeforeDamage = Health;
		HUDMaxBeforeDamageHealth = MaxHealth;
	}
}


void ABlasterPlayerController::SetHUDShieldNative(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBarNative;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		if (UMaterialInstanceDynamic* ShieldBarMaterial = BlasterHUD->CharacterOverlay->HealthBarNative->GetDynamicMaterial())
		{
			ShieldBarMaterial->SetScalarParameterValue(FName("Shield"), ShieldPercent);
		}
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}


void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&	// HUD存在
			BlasterHUD->CharacterOverlay &&	// HUD的角色信息UI存在
			BlasterHUD->CharacterOverlay->ShieldBar_R2L &&	// HUD的角色信息UI的护盾条存在(从右到左)
			BlasterHUD->CharacterOverlay->ShieldBar_L2R &&	// HUD的角色信息UI的护盾条存在(从左到右)
			BlasterHUD->CharacterOverlay->ShieldText;	// HUD的角色信息UI的护盾值文本存在

	if (bHUDValid)
	{
		float ShieldPercent = Shield / MaxShield;
		
		// 如果我们的护盾值小于等于当前血条的反向值，那么护盾值的起始点就是血条的终点（为了实现这种效果，我将护盾的百分比加上血条百分比）
		if (BlasterHUD->CharacterOverlay->HealthBar->Percent < 1.f && ShieldPercent > 0 && ShieldPercent <= 1.f - BlasterHUD->CharacterOverlay->HealthBar->Percent)
		{
			ShieldPercent += BlasterHUD->CharacterOverlay->HealthBar->Percent;
			// 然后我们使用从左到右的进度条填充类型
			BlasterHUD->CharacterOverlay->ShieldBar_R2L->SetPercent(0);
			BlasterHUD->CharacterOverlay->ShieldBar_L2R->SetPercent(ShieldPercent);
		}
		else if (BlasterHUD->CharacterOverlay->HealthBar->Percent < 1.f && ShieldPercent > 0 && ShieldPercent > 1.f - BlasterHUD->CharacterOverlay->HealthBar->Percent)
		{
			// 然后我们使用从右到左的进度条填充类型
			BlasterHUD->CharacterOverlay->ShieldBar_R2L->SetPercent(ShieldPercent);
			BlasterHUD->CharacterOverlay->ShieldBar_L2R->SetPercent(0);

		}
		else if (BlasterHUD->CharacterOverlay->HealthBar->Percent == 1.f)
		{
			// 如果我们的血条是满的，那么护盾的进度条的填充类型就是从右到左
			BlasterHUD->CharacterOverlay->ShieldBar_R2L->SetPercent(ShieldPercent);
			BlasterHUD->CharacterOverlay->ShieldBar_L2R->SetPercent(0);
		}

		if (ShieldPercent <= 0.f)
		{
			BlasterHUD->CharacterOverlay->ShieldBar_L2R->SetPercent(0);
			BlasterHUD->CharacterOverlay->ShieldBar_R2L->SetPercent(0);
		}

		FString ShieldString = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldString));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60.f;

		FString CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->Announcement && BlasterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes * 60.f;

		const FString CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	const bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		const FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;

	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTeardown();
		}
	}

}

void ABlasterPlayerController::BroadcastElimAnnouncement(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			if (BlasterHUD->AnnouncementClass)
			{
				if (Attacker == Self && Victim != Self)
				{
					BlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				}
				else if (Victim == Self && Attacker != Self)
				{
					BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				}
				else if (Attacker == Self && Victim == Self)
				{
					BlasterHUD->AddElimAnnouncement("You", "yourself");
				}
				else if (Attacker != Self && Victim != Self && Attacker == Victim)
				{
					BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				}
				else
				{
					BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
				}
			}
		}
	}

	
}
