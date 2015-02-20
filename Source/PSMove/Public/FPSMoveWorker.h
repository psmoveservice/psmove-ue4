//
//  FPSMoveWorker.h
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-20.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#pragma once

class FPSMoveWorker : public FRunnable
{
public:
    FPSMoveWorker(FVector& PSMovePos, FQuat& PSMoveOrientation);
    virtual ~FPSMoveWorker(); // Why is this virtual?

    /** Thread for polling the controller and tracker */
    FRunnableThread* Thread;

    /** Ptr to data containing position and orientation. */
    FQuat* WorkerOrientation;
    FVector* WorkerPosition;

    /** Objects needed by the Thread
     *  i.e., move_controllers and tracker
     */

    /** Thread Safe Counter. ?? */
    FThreadSafeCounter StopTaskCounter;

    /** FRunnable Interface */
    virtual bool Init(); // override?
    virtual uint32 Run(); // override?
    virtual void Stop(); // override?

    /** Singleton instance for static access. */
    static FPSMoveWorker* WorkerInstance;
    /** Static access to start the thread.*/
    static FPSMoveWorker* PSMoveWorkerInit(FVector& PSMovePos, FQuat& PSMoveOrientation);
    /** Static access to stop the thread.*/
    static void Shutdown();

private:
    int m_move_count;
    PSMove **m_moves;
    PSMoveTracker *m_tracker;
    float m_tracker_width;
    float m_tracker_height;
};