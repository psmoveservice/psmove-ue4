#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include "IPSMove.h"
#include "FPSMove.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

void FPSMove::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    UE_LOG(LogPSMove, Log, TEXT("Loaded PSMove module."));

}

void FPSMovePlugin::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // If we had to do any work to load the library in StartupModule, we would undo that work here.
    // UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));
}
