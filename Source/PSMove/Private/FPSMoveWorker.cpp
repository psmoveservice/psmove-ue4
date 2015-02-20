//
//  FPSMoveWorker.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-20.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#include "PSMovePrivatePCH.h"
#include "FPSMoveWorker.h"
#include "IPSMove.h"

    //
FPSMoveWorker* FPSMoveWorker::WorkerInstance = NULL;

FPSMoveWorker::FPSMoveWorker(FVector& PSMovePosition, FQuat& PSMoveOrientation)
    : StopTaskCounter(0)
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker::FPSMoveWorker"));
    WorkerPosition = &PSMovePosition;
    WorkerOrientation = &PSMoveOrientation;

    m_move_count = psmove_count_connected();
    UE_LOG(LogPSMove, Log, TEXT("Found %d PSMove controllers."), m_move_count);
    m_moves = (PSMove**)calloc(m_move_count, sizeof(PSMove*));
    m_tracker = psmove_tracker_new();
    psmove_tracker_set_exposure(m_tracker, Exposure_LOW);

    int width, height;
    psmove_tracker_get_size(m_tracker, &width, &height);
    m_tracker_width = (float)width;
    m_tracker_height = (float)height;

    for (int i = 0; i<m_move_count; i++)
    {
        m_moves[i] = psmove_connect_by_id(i);
        psmove_enable_orientation(m_moves[i], PSMove_True);
        assert(psmove_has_orientation(m_moves[i]));

        while (psmove_tracker_enable(m_tracker, m_moves[i]) != Tracker_CALIBRATED);
            //TODO: psmove_tracker_enable_with_color(m_tracker, m_moves[i], r, g, b)
            //TODO: psmove_tracker_get_color(m_tracker, m_moves[i],unisgned char &r, &g, &b);
        enum PSMove_Bool auto_update_leds = psmove_tracker_get_auto_update_leds(m_tracker, m_moves[i]);
    }

    // I think this auto-inits and runs the thread.
    Thread = FRunnableThread::Create(this, TEXT("FPSMoveWorker"), 0, TPri_AboveNormal);
}

FPSMoveWorker::~FPSMoveWorker()
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMove Destructor"));

    for (int i = 0; i<m_move_count; i++)
    {
        psmove_disconnect(m_moves[i]);
    }
    psmove_tracker_free(m_tracker);
    free(m_moves);

    delete Thread;
    Thread = NULL;

}

bool FPSMoveWorker::Init()
{
    return true;
}

uint32 FPSMoveWorker::Run()
{
    // TEMP - Local variables until I pass to PSMoveQuat.
    float px, py, pr, pd;
    int buttons;
    unsigned int pressed, released;

    //Initial wait before starting.
    FPlatformProcess::Sleep(0.03);

    while (StopTaskCounter.GetValue() == 0)
    {
        //Poll controller, camera, and update PSMovePos and PSMoveQuat.

        // Renew the image on camera
        psmove_tracker_update_image(m_tracker); // Sometimes libusb crashes here.
        psmove_tracker_update(m_tracker, NULL); // Passing null (instead of m_moves[i]) updates all controllers.

        for (int i = 0; i < m_move_count; i++)
        {
            psmove_tracker_get_position(m_tracker, m_moves[i], &px, &py, &pr);
            py = m_tracker_height - py;
            //TODO: Use my own distance_from_radius function.
            pd = psmove_tracker_distance_from_radius(m_tracker, pr);
            WorkerPosition->Set(px, py, pd);

            psmove_poll(m_moves[i]); // Required to get orientation.
            
            //I suppose it is possible that W,X,Y,Z do not get updated at the exact same moment.
            psmove_get_orientation(m_moves[i],
                                   &WorkerOrientation->W,
                                   &WorkerOrientation->X,
                                   &WorkerOrientation->Y,
                                   &WorkerOrientation->Z);

            //UE_LOG(LogPSMove, Log, TEXT("Controller %d: pos = %f %f px, z=%f cm; orientation wxyz = %f %f %f %f"), px, py, pd, ow, ox, oy, oz);

            buttons = psmove_get_buttons(m_moves[i]);
            psmove_get_button_events(m_moves[i], &pressed, &released);  // i.e., state change

            // e.g., pressed & Btn_CROSS or buttons & Btn_MOVE
        }

        FPlatformProcess::Sleep(0.02);

    }

    return 0;
}

void FPSMoveWorker::Stop()
{
    StopTaskCounter.Increment();
}

FPSMoveWorker* FPSMoveWorker::PSMoveWorkerInit(FVector& PSMovePosition, FQuat& PSMoveOrientation)
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker::PSMoveWorkerInit"));
    if (!WorkerInstance && FPlatformProcess::SupportsMultithreading())
    {
        UE_LOG(LogPSMove, Log, TEXT("Creating new WorkerInstance"));
        WorkerInstance = new FPSMoveWorker(PSMovePosition, PSMoveOrientation);
    }
    return WorkerInstance;
}

void FPSMoveWorker::Shutdown()
{
    if (WorkerInstance)
    {
        UE_LOG(LogPSMove, Log, TEXT("Shutting down WorkerInstance."));
        WorkerInstance->Stop();
        WorkerInstance->Thread->WaitForCompletion();
        delete WorkerInstance; // Destructor SHOULD turn off tracker.
        WorkerInstance = NULL;
    }
}