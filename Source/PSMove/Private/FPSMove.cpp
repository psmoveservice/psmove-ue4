#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "FPSMove.h"
#include "FPSMoveWorker.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

void FPSMove::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
    UE_LOG(LogPSMove, Log, TEXT("Loading PSMove module..."));
    UE_LOG(LogPSMove, Log, TEXT("Initializing PSMoveWorker Thread..."));
    FPSMoveWorker::InitializeSingleton();
}

void FPSMove::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    
    UE_LOG(LogPSMove, Log, TEXT("Shutting down PSMove module."));
    FPSMoveWorker::ShutdownSingleton();
}

bool FPSMove::AcquirePSMove(int32 PSMoveID, FPSMoveDataContext *DataContext)
{
	bool success = false;
	FPSMoveWorker* WorkerSingleton = FPSMoveWorker::GetSingletonInstance();

	if (WorkerSingleton != NULL)
	{
		success = WorkerSingleton->AcquirePSMove(PSMoveID, DataContext);
	}

	return success;
}

void FPSMove::ReleasePSMove(FPSMoveDataContext *DataContext)
{
    FPSMoveWorker* WorkerSingleton = FPSMoveWorker::GetSingletonInstance();
    if (WorkerSingleton != NULL)
    {
        WorkerSingleton->ReleasePSMove(DataContext);
    }
}