#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "IPSMove.h"
#include "FPSMove.h"
#include "FPSMoveWorker.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

FPSMove::FPSMove()
{

}

void FPSMove::StartupModule()
{
    UE_LOG(LogPSMove, Log, TEXT("Loading PSMove module..."));
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
    // Init the PSMoveWorker singleton. This will also init the controllers and tracker.
    UE_LOG(LogPSMove, Log, TEXT("Initializing PSMoveWorker..."));
    FPSMoveWorker::PSMoveWorkerInit(ModulePosition, ModuleOrientation);
}

void FPSMove::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // TODO: Kill the PSMoveWorker. That will also disconnect the controllers and tracker.
    FPSMoveWorker::Shutdown();
    UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));

}

const FVector FPSMove::GetPosition() const
{
    return ModulePosition;
}

const FQuat FPSMove::GetOrientation() const
{
    return ModuleOrientation;
}