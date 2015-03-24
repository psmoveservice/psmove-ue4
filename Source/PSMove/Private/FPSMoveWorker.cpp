//
//  FPSMoveWorker.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-20.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#include "PSMovePrivatePCH.h"
#include "FPSMoveWorker.h"
#include "FPSMove.h"
#include <math.h>
#include <assert.h>

    //
FPSMoveWorker* FPSMoveWorker::WorkerInstance = NULL;

FPSMoveWorker::FPSMoveWorker(TArray<FVector>& PSMovePositions, TArray<FQuat>& PSMoveOrientations, TArray<uint32>& PSMoveButtons, TArray<uint32>& PSMovePressed, TArray<uint32>& PSMoveReleased, TArray<uint8>& PSMoveTriggers)  //, TArray<uint8>& PSMoveRumbleRequests
    : StopTaskCounter(0)
{
    WorkerPositions = &PSMovePositions;
    WorkerOrientations = &PSMoveOrientations;
    WorkerButtons = &PSMoveButtons;
    WorkerPressed = &PSMovePressed;
    WorkerReleased = &PSMoveReleased;
    WorkerTriggers = &PSMoveTriggers;
    //WorkerRumbleRequests = &PSMoveRumbleRequests;

    m_move_count = psmove_count_connected();
    m_moves = (PSMove**)calloc(m_move_count, sizeof(PSMove*));
    UE_LOG(LogPSMove, Log, TEXT("Found %d PSMove controllers."), m_move_count);
    m_tracker = psmove_tracker_new(); // Unfortunately the API does not have a way to change the resolution and framerate.
    
    if (m_move_count>0)
    {
        for (int i = 0; i<m_move_count; i++)
        {
            m_moves[i] = psmove_connect_by_id(i);
            psmove_enable_orientation(m_moves[i], PSMove_True);
            assert(psmove_has_orientation(m_moves[i]));
            WorkerOrientations->Add(FQuat(0.0, 0.0, 0.0, 1.0));
            WorkerButtons->Add(0);
            WorkerPressed->Add(0);
            WorkerReleased->Add(0);
            WorkerTriggers->Add(0);
            //WorkerRumbleRequests->Add(0);
        }

        if (m_tracker)
        {
            psmove_tracker_set_exposure(m_tracker, Exposure_MEDIUM);
            int width, height;
            psmove_tracker_get_size(m_tracker, &width, &height);
            m_tracker_width = (float)width;
            m_tracker_height = (float)height;
            UE_LOG(LogPSMove, Log, TEXT("Camera Dimensions: %f x %f"), m_tracker_width, m_tracker_height);

            for (int i = 0; i<m_move_count; i++)
            {
                while (psmove_tracker_enable(m_tracker, m_moves[i]) != Tracker_CALIBRATED);
                //TODO: psmove_tracker_enable_with_color(m_tracker, m_moves[i], r, g, b)
                //TODO: psmove_tracker_get_color(m_tracker, m_moves[i],unisgned char &r, &g, &b);
                enum PSMove_Bool auto_update_leds = psmove_tracker_get_auto_update_leds(m_tracker, m_moves[i]);
                WorkerPositions->Add(FVector(0.0));
            }
        }
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
    float xpx, ypx, rpx;
    float xcm, ycm, zcm;
    int buttons;
    unsigned int pressed, released;
    //float pr_conv = (M_PI/180.0) * (75.0/800.0); // 75 deg per 800 px

    //Initial wait before starting.
    FPlatformProcess::Sleep(0.03);

    while (StopTaskCounter.GetValue() == 0 && m_moves > 0)
    {
        //Poll controller, camera, and update PSMovePos and PSMoveQuat.

        // Renew the image on camera
        if (m_tracker)
        {
            psmove_tracker_update_image(m_tracker); // Sometimes libusb crashes here.
            psmove_tracker_update(m_tracker, NULL); // Passing null (instead of m_moves[i]) updates all controllers.

            for (int i = 0; i < m_move_count; i++)
            {
                psmove_tracker_get_position(m_tracker, m_moves[i], &xpx, &ypx, &rpx);
                //zero x and y on camera's principal axis
                xpx = xpx - (m_tracker_width/2.0);  // Zero x on camera's principal axis
                ypx = (m_tracker_height/2.0) - ypx;  // Zero y on camera's principal axis. ypx starts as pixels from top.
                zcm = psmove_tracker_distance_from_radius(m_tracker, rpx);  // Use Thomas' parametric fit to get depth in cm
                //zcm = 2.25 / sin( pr_conv * rpx ); //Use trigonometry to get depth in cm
                // We know sphere radius = 2.25 cm, which gives us pixel->cm conversion factor at this depth.
                xcm = xpx * 2.25 / rpx;
                ycm = ypx * 2.25 / rpx;

                //UE_LOG(LogPSMove, Log, TEXT("Pixels: %f %f %f --> cm: %f %f %f"), xpx, ypx, rpx, xcm, ycm, zcm);
                (*WorkerPositions)[i].Set(-zcm, -xcm, ycm); // In UE4, up/down (gravity dir) is z

                /**
                 * psmoveapi's fusion classes do the following to transform these results into coordinates:
                 * projection = glm::perspectiveFov(fov, m_tracker_width, m_tracker_height, 0.1, 1000.0)
                 * viewport = glm::vec4(0., 0., m_tracker_width, m_tracker_height);
                 * modelview = glm::translate(glm::mat4(), glm::vec3(x, y, z)) * glm::mat4_cast(quaternion);
                 */
            }
        } else {
            FPlatformProcess::Sleep(0.004);
        }

        for (int i = 0; i < m_move_count; i++)
        {
            //TODO: Apparently hidapi, called by psmove_poll, gets the oldest frame from the device.
            //Is it necessary to keep polling until no frames are left?
            while (psmove_poll(m_moves[i]) > 0)
            {
                // Update the controller status (via bluetooth)
                psmove_poll(m_moves[i]);
                
                // Get the controller orientation (uses IMU).
                psmove_get_orientation(m_moves[i],
                                   &((*WorkerOrientations)[i].W),
                                   &((*WorkerOrientations)[i].X),
                                   &((*WorkerOrientations)[i].Y),
                                   &((*WorkerOrientations)[i].Z));
                //I suppose it is possible that W,X,Y,Z do not get updated at the exact same moment.
                
                // Get the controller button state
                (*WorkerButtons)[i] = psmove_get_buttons(m_moves[i]);  // Bitwise, if each button is down.
                psmove_get_button_events(m_moves[i], &( (*WorkerPressed)[i] ), &( (*WorkerReleased)[i] ));  // i.e., state change
                
                // Get the controller trigger value (uint8; 0-255)
                uint8 temp = psmove_get_trigger(m_moves[i]);
                (*WorkerTriggers)[i] = temp;
                
                // Set the controller rumble (uint8; 0-255)
                //psmove_set_rumble(m_moves[i], (*WorkerRumbleRequests)[i]);
            }
        }

        //Sleeping the thread seems to crash libusb.
        //FPlatformProcess::Sleep(0.005);

    }

    return 0;
}

void FPSMoveWorker::Stop()
{
    StopTaskCounter.Increment();
}

FPSMoveWorker* FPSMoveWorker::PSMoveWorkerInit(TArray<FVector>& PSMovePositions, TArray<FQuat>& PSMoveOrientations, TArray<uint32>& PSMoveButtons, TArray<uint32>& PSMovePressed, TArray<uint32>& PSMoveReleased, TArray<uint8>& PSMoveTriggers) //TArray<uint8>& PSMoveRumbleRequests
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker::PSMoveWorkerInit"));
    if (!WorkerInstance && FPlatformProcess::SupportsMultithreading())
    {
        UE_LOG(LogPSMove, Log, TEXT("Creating new FPSMoveWorker instance."));
        WorkerInstance = new FPSMoveWorker(PSMovePositions, PSMoveOrientations, PSMoveButtons, PSMovePressed, PSMoveReleased, PSMoveTriggers); //PSMoveRumbleRequests
    } else {
        UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker already instanced."));
    }
    return WorkerInstance;
}

void FPSMoveWorker::Shutdown()
{
    if (WorkerInstance)
    {
        UE_LOG(LogPSMove, Log, TEXT("Shutting down PSMoveWorker instance."));
        WorkerInstance->Stop();
        WorkerInstance->Thread->WaitForCompletion();
        delete WorkerInstance; // Destructor SHOULD turn off tracker.
        WorkerInstance = NULL;
        UE_LOG(LogPSMove, Log, TEXT("PSMoveWorker instance destroyed."));
    }
}