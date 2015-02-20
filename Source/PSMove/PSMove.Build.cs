// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
    public class PSMove : ModuleRules
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

        public PSMove(TargetInfo Target)
        {
            PublicIncludePaths.AddRange(
                new string[] {
                    "PSMove/Public",
                    "PSMove/Classes",
                    // ... add public include paths required here ...
                }
            );

            PrivateIncludePaths.AddRange(
                new string[] {
                    "PSMove/Private",
                    Path.Combine(ThirdPartyPath, "psmoveapi", "include"),
                    // ... add other private include paths required here ...
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine",               // Used by Actor
                    "Core",
                    "CoreUObject",          // Actors and Structs
                    "InputCore",            // Provides LOCTEXT and other Input features
                    "HeadMountedDisplay"    // Eventually we may want to have the plugin contain its own calibration methods.
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

        public bool LoadPsmoveapi(TargetInfo Target)
        {
            bool isLibrarySupported = false;
            string PlatformString = "Mac";
            if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
            {
                isLibrarySupported = true;
                PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
                PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "libpsmoveapi_static.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "libpsmoveapi_tracker_static.a"));

            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                isLibrarySupported = true;
                PlatformString = "Mac";
                //Macs load binaries slightly differently. Maybe need to use BinariesPath?
                PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, PlatformString, "libpsmoveapi.dylib"));
                PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, PlatformString, "libpsmoveapi_tracker.dylib"));
            }

            return isLibrarySupported;
        }
    }
}