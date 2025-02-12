// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class VisualizationServer : ModuleRules
{
	public VisualizationServer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking", "Json", "JsonUtilities" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		string VcpkgRoot = System.Environment.GetEnvironmentVariable("VCPKG_ROOT");
		if (!string.IsNullOrEmpty(VcpkgRoot))
		{
			string VcpkgIncludePath = Path.Combine(VcpkgRoot, "installed", "x64-windows", "include");
			PublicIncludePaths.Add(VcpkgIncludePath);
			System.Console.WriteLine("Using vcpkg include path: " + VcpkgIncludePath);
		}
		else
		{
			System.Console.WriteLine("VCPKG_ROOT environment variable is not set!");
		}
		
		
		ShadowVariableWarningLevel = WarningLevel.Warning;
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
