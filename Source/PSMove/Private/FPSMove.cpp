#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "FPSMove.h"
#include "FPSMoveWorker.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

void FPSMove::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
    UE_LOG(LogPSMove, Log, TEXT("Loading PSMove module..."));
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
    FPSMoveWorker::PSMoveWorkerInit();
}


bool FPSMove::AcquirePSMove(
	int32 PSMoveID,
	FPSMoveDataContext *DataContext)
{
	bool success = false;
	FPSMoveWorker* WorkerSingleton = FPSMoveWorker::GetSingletonInstance();

	if (WorkerSingleton != NULL)
	{
		success = WorkerSingleton->AcquirePSMove(PSMoveID, DataContext);
	}

	return success;
}