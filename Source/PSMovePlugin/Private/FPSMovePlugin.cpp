#include "PSMovePluginPrivatePCH.h"  // Also includes psmove api lib headers.

#include "IPSMovePlugin.h"
#include "FPSMovePlugin.h"

IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )

void FPSMovePlugin::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    UE_LOG(LogPSMovePlugin, Log, TEXT("Loaded PSMove plugin"));

}

void FPSMovePlugin::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // If we had to do any work to load the library in StartupModule, we would undo that work here.
    // UE_LOG(LogPSMovePlugin, Log, TEXT("Shutdown PSMove plugin."));
}
