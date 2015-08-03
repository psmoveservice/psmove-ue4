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
	static const int k_max_controllers = 5; // 5 tracking colors available: magenta, cyan, yellow, red, blue

	/** Static access to get the worker singleton.*/
	static FPSMoveWorker* GetSingletonInstance() { return WorkerInstance; }
	/** Static access to start the thread.*/
	static FPSMoveWorker* PSMoveWorkerInit();
	/** Static access to stop the thread.*/
	static void Shutdown();

	/**
	* Tell the PSMove Worker that we want to start listening to this controller.
	*
	* @return True if we can successfully acquire the controller.
	*/
	bool AcquirePSMove(int32 PSMoveID, FPSMoveDataContext *DataContext);

    /** Request the Worker check to see if the number of controllers has changed. */
    void RequestMoveCheck();

private:
	FPSMoveWorker(); // Called by singleton access via Init
	virtual ~FPSMoveWorker();

	/** FRunnable Interface */
	virtual bool Init(); // override?
	virtual uint32 Run(); // override?
	virtual void Stop(); // override?

private:
	/** Singleton instance for static access. */
	static FPSMoveWorker* WorkerInstance;

	/** Thread for polling the controller and tracker */
	FRunnableThread* Thread;

	/** Thread Safe Counter. */
	FThreadSafeCounter StopTaskCounter;

	/** An array of raw data structures, one for each controller */
	FPSMoveRawControllerData_TLS WorkerControllerDataArray[k_max_controllers];

	/** 
	 * Published worker data that shouldn't touched directly.
	 * Access through _TLS version of the structures. 
	 */
	FPSMoveRawControllerData_Concurrent WorkerControllerDataArray_Concurrent[k_max_controllers];

    uint8 PSMoveCount;
	uint32 LastMoveCountCheckTime;
	bool UpdateControllerConnections(PSMoveTracker *Tracker, PSMove **psmoves);
};