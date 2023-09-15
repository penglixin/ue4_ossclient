// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
public class ossclientTarget : TargetRules
{
	public ossclientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Program;
		LinkType = TargetLinkType.Monolithic;
		LaunchModuleName = "ossclient";

		// Lean and mean
		bBuildDeveloperTools = false;

		// Never use malloc profiling in Unreal Header Tool.  We set this because often UHT is compiled right before the engine
		// automatically by Unreal Build Tool, but if bUseMallocProfiler is defined, UHT can operate incorrectly.
		bUseMallocProfiler = false;

		// Editor-only data, however, is needed
		bBuildWithEditorOnlyData = true;

		// Currently this app is not linking against the engine, so we'll compile out references from Core to the rest of the engine
		bCompileAgainstEngine = false;  // 是否链接所有引擎程序生成的项目
		bCompileAgainstCoreUObject = true; // 需要连接到CoreUObject模块
		bCompileAgainstApplicationCore = true;

		// UnrealHeaderTool is a console application, not a Windows app (sets entry point to main(), instead of WinMain())
		bIsBuildingConsoleApplication = false;// true:控制台app;false:窗体app
	}
}
