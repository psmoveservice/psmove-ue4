//
//  FPSMoveWorker.h
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-20.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#pragma once
#include "FPSMoveClock.h"
#include "PSMovePrivatePCH.h"

struct FPSMoveRawControllerWorkerData_TLS : public FPSMoveRawControllerData_TLS
{
    FPSMoveControllerClock Clock;
    inline void Clear()
    {
        FPSMoveRawControllerData_TLS::Clear();
        Clock.Initialize();
    }
};

class FPSMoveWorker : public FRunnable
{
public:
    static const int k_max_controllers = 5; // 5 tracking colors available: magenta, cyan, yellow, red, blue

    /** Static access to get the worker singleton.*/
    static FPSMoveWorker* GetSingletonInstance() { return WorkerInstance; }
    /** Static access to start the thread.*/
    static FPSMoveWorker* InitializeSingleton();
    /** Static access to stop the thread and destroy the worker instance.*/
    static void ShutdownSingleton();

    /**
    * Tell the PSMove Worker that we want to start listening to this controller.
    *
    * @return True if we can successfully acquire the controller.
    */
    bool AcquirePSMove(int32 PSMoveID, FPSMoveDataContext *DataContext);
    
    /** Tell the PSMove Worker that we don't care about listening to this controller anymore. */
    void ReleasePSMove(FPSMoveDataContext *DataContext);

private:
    FPSMoveWorker(); // Called by singleton access via Init
    virtual ~FPSMoveWorker();

    /** FRunnable Interface */
    virtual bool Init() override;
    virtual uint32 Run(); // override?
    virtual void Stop() override;  // Increments the stop counter, stopping the thread.

private:
    /** Singleton instance for static access. */
    static FPSMoveWorker* WorkerInstance;

    /** Thread for polling the controller and tracker */
    FRunnableThread* Thread;
    
    /** Thread Safe Counter. */
    FThreadSafeCounter AcquiredContextCounter;

    /** Thread Safe Counter. When incremented by Stop (which is called only by ShutdownSingleton)*/
    FThreadSafeCounter StopTaskCounter;
    
    /** Shared data shared amongst all controllers  (i.e. configuration data)  */
    FPSMoveRawSharedData_TLS WorkerSharedData;

    /** An array of raw data structures, one for each controller */
    FPSMoveRawControllerWorkerData_TLS WorkerControllerDataArray[k_max_controllers];

    /** 
     * Published worker data that shouldn't be touched directly.
     * Access through _TLS version of the structures. 
     */
    FPSMoveRawSharedData_Concurrent WorkerSharedData_Concurrent;
    FPSMoveRawControllerData_Concurrent WorkerControllerDataArray_Concurrent[k_max_controllers];
};