// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;	// UnrealBuildTool is a set of scripts used to build Unreal Engine 4 (UE4) across multiple platforms.
using System.Collections.Generic;	// System.Collections.Generic contains interfaces and classes that define generic collections, which allow users to create strongly typed collections that provide better type safety and performance than non-generic strongly typed collections.

public class BlasterTarget : TargetRules
{
	public BlasterTarget( TargetInfo Target) : base(Target)     // TargetInfo是一个包含了目标平台，比如平台名称、平台架构、编译器等等信息的结构体
    {
		Type = TargetType.Game;		// TargetType是一个枚举类型，包含了Game、Client、Server、Editor、Program、Unspecified等类型
		DefaultBuildSettings = BuildSettingsVersion.V2;		// BuildSettingsVersion是一个枚举类型，包含了V1、V2、V3等类型
		ExtraModuleNames.AddRange( new string[] { "Blaster" } );	// ExtraModuleNames是一个字符串数组，用于指定需要编译的模块名称
	}
}
