#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_RedTeam UMETA(DisplayName = "Red Team"),
	ET_BlueTeam UMETA(DisplayName = "Blue Team"),
	ET_GreenTeam UMETA(DisplayName = "Green Team"),
	ET_YellowTeam UMETA(DisplayName = "Yellow Team"),
	ET_PurpleTeam UMETA(DisplayName = "Purple Team"),
	ET_OrangeTeam UMETA(DisplayName = "Orange Team"),
	ET_PinkTeam UMETA(DisplayName = "Pink Team"),
	ET_BlackTeam UMETA(DisplayName = "Black Team"),
	ET_WhiteTeam UMETA(DisplayName = "White Team"),

	ET_NoTeam UMETA(DisplayName = "No Team"),

	ET_MAX UMETA(DisplayName = "DefaultMax")
};
