// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;	// UnrealBuildTool is a set of scripts used to build Unreal Engine 4 (UE4) across multiple platforms.
using System.Collections.Generic;	// System.Collections.Generic contains interfaces and classes that define generic collections, which allow users to create strongly typed collections that provide better type safety and performance than non-generic strongly typed collections.

public class BlasterTarget : TargetRules
{
	public BlasterTarget( TargetInfo Target) : base(Target)     // TargetInfo��һ��������Ŀ��ƽ̨������ƽ̨���ơ�ƽ̨�ܹ����������ȵ���Ϣ�Ľṹ��
    {
		Type = TargetType.Game;		// TargetType��һ��ö�����ͣ�������Game��Client��Server��Editor��Program��Unspecified������
		DefaultBuildSettings = BuildSettingsVersion.V2;		// BuildSettingsVersion��һ��ö�����ͣ�������V1��V2��V3������
		ExtraModuleNames.AddRange( new string[] { "Blaster" } );	// ExtraModuleNames��һ���ַ������飬����ָ����Ҫ�����ģ������
	}
}
