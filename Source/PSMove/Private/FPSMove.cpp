#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include "IPSMove.h"
#include "FPSMove.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

void FPSMove::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    UE_LOG(LogPSMove, Log, TEXT("Loaded PSMove module."));

}

void FPSMove::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // If we had to do any work to load the library in StartupModule, we would undo that work here.
    UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));
}

// Activate the device. Turn on the tracker. Setup fusion.
void FPSMove::MoveSetup()
{
    UE_LOG(LogPSMove, Log, TEXT("TODO: activate move, activate tracker, activate fusion."));
}

// Deactivate the device. Turn off the tracker. Unset fusion.
void FPSMove::MoveUnset()
{
    UE_LOG(LogPSMove, Log, TEXT("TODO: Deactivate move, deactivate tracker, deactivate fusion."));
}

// (V1) Polls device. Calculates position and orientation. Fires events.
void FPSMove::MoveTick(float DeltaTime)
{
    UE_LOG(LogPSMove, Log, TEXT("TODO: poll, data, fire events."));
}