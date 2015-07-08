//
//  FPSMoveWorker.h
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-20.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#pragma once
#include "PSMovePrivatePCH.h"

class FPSMoveWorker : public FRunnable
{
public:
    FPSMoveWorker(TArray<FPSMoveRawDataFrame>* &PSMoveRawDataArrayPtr); // Usually called by singleton access via Init
    virtual ~FPSMoveWorker();

    /** Thread for polling the controller and tracker */
    FRunnableThread* Thread;

    /** Thread Safe Counter. ?? */
    FThreadSafeCounter StopTaskCounter;
    
    /** An array of raw data strctures, one for each controller */
    TArray<FPSMoveRawDataFrame> WorkerDataFrames;

    /** Request the Worker check to see if the number of controllers has changed. */
    void RequestMoveCheck();

    /** FRunnable Interface */
    virtual bool Init(); // override?
    virtual uint32 Run(); // override?
    virtual void Stop(); // override?

    /** Singleton instance for static access. */
    static FPSMoveWorker* WorkerInstance;
    /** Static access to start the thread.*/
    static FPSMoveWorker* PSMoveWorkerInit(TArray<FPSMoveRawDataFrame>* &PSMoveRawDataArrayPtr);
    /** Static access to stop the thread.*/
    static void Shutdown();

private:
    /** We need a reference to the Module's RawDataArrayPtr because it will be passed in on initialization but not used until the Thread runs. */
    TArray<FPSMoveRawDataFrame>** ModuleRawDataArrayPtrPtr;
    uint8 PSMoveCount;
    bool MoveCheckRequested;
    bool UpdateMoveCount();
};