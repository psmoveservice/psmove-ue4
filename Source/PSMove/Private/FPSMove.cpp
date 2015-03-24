#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "FPSMove.h"
#include "FPSMoveWorker.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

void FPSMove::StartupModule()
{
    UE_LOG(LogPSMove, Log, TEXT("Loading PSMove module..."));
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}

void FPSMove::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // TODO: Kill the PSMoveWorker. That will also disconnect the controllers and tracker.
    FPSMoveWorker::Shutdown();
    UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));
}

void FPSMove::InitWorker()
{
    // Init the PSMoveWorker singleton if needed.
    // This will also init the controllers and tracker if needed.
    UE_LOG(LogPSMove, Log, TEXT("Trying to initializing PSMoveWorker..."));
    FPSMoveWorker::PSMoveWorkerInit(ModulePositions, ModuleOrientations, ModuleButtons, ModulePressed, ModuleReleased);
}

const FVector FPSMove::GetPosition(uint8 PSMoveID) const
{
    if (ModulePositions.IsValidIndex(PSMoveID))
    {
        return ModulePositions[PSMoveID];
    } else {
        return FVector(0.0);
    }
}

const FQuat FPSMove::GetOrientation(uint8 PSMoveID) const
{
    if (ModuleOrientations.IsValidIndex(PSMoveID))
    {
        return ModuleOrientations[PSMoveID];
    } else {
        return FQuat(0.0, 0.0, 0.0, 1.0);
    }
}

const uint32 FPSMove::GetButtons(uint8 PSMoveID) const
{
    if (ModuleButtons.IsValidIndex(PSMoveID))
    {
        return ModuleButtons[PSMoveID];
    } else {
        return 0;
    }
}

const uint32 FPSMove::GetPressed(uint8 PSMoveID) const
{
    if (ModulePressed.IsValidIndex(PSMoveID))
    {
        return ModulePressed[PSMoveID];
    } else {
        return 0;
    }
}

const uint32 FPSMove::GetReleased(uint8 PSMoveID) const
{
    if (ModuleReleased.IsValidIndex(PSMoveID))
    {
        return ModuleReleased[PSMoveID];
    } else {
        return 0;
    }
}