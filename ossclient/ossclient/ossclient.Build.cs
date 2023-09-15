// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ossclient : ModuleRules
{
	public ossclient(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("Runtime/Launch/Public");

		PrivateIncludePaths.Add("Runtime/Launch/Private");		// For LaunchEngineLoop.cpp include

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("Projects");
		PrivateDependencyModuleNames.Add("Slate");
		PrivateDependencyModuleNames.Add("SlateCore");
		PrivateDependencyModuleNames.Add("StandaloneRenderer");


		PrivateDependencyModuleNames.Add("Json");
		PrivateDependencyModuleNames.Add("OSSAPI");
	}
}
