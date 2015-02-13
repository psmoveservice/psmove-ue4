// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class PSMovePlugin : ModuleRules
	{
		
		private string ModulePath
		{
			get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
		}
		private string ThirdPartyPath{
			get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty")); }
		}
		private string BinariesPath
		{
			get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/")); }
		}
		private string IncludPath
		{
			get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "psmoveapi", "include")); }
		}
		private string LibraryPath
		{
			get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "psmoveapi", "lib")); }
		}
		
		public PSMovePlugin(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					"PSMovePlugin/Public",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"PSMovePlugin/Private",
					Path.Combine(ThirdPartyPath, "psmoveapi", "include"),
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Engine",
					"Core",
					"CoreUObject",
					"InputCore",
					"HeadMountedDisplay"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
				
			LoadPsmoveapi(Target);
		}
		
		public void LoadPsmoveapi(TargetInfo Target)
		{
            string PlatformString = "Mac";"
			if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
			{
				PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac){
                PlatformString = "Mac";
            }

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "libpsmoveapi_static.a"));
            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "libpsmoveapi_tracker_static.a"));

            //Macs load binaries slightly differently. Maybe need to use BinariesPath?

		}
	}
}