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

FPSMoveWorker::FPSMoveWorker(TArray<FPSMoveRawDataFrame>* &PSMoveRawDataArrayPtr)
    : StopTaskCounter(0), PSMoveCount(0), MoveCheckRequested(false)
{
    // Keep a reference to the Module's DataFrameArrayPtr
    ModuleRawDataArrayPtrPtr = &PSMoveRawDataArrayPtr;
    
    // Need to count the number of controllers before starting the thread to guarnatee the data arrays exist before returning.
    bool updated = UpdateMoveCount();
    
    // This Inits and Runs the thread.
    Thread = FRunnableThread::Create(this, TEXT("FPSMoveWorker"), 0, TPri_AboveNormal);
}

bool FPSMoveWorker::UpdateMoveCount()
{
    MoveCheckRequested = false;
    int newcount = psmove_count_connected();
    UE_LOG(LogPSMove, Log, TEXT("Found %d PSMove controllers."), newcount);
    if (PSMoveCount != newcount)
    {
        PSMoveCount = newcount;
        // Instantiate the local array of raw data frames.
        WorkerDataFrames.SetNumZeroed(PSMoveCount);
        // Set the module's ModuleRawDataArrayPtr to point to this thread's array of raw data frames.
        (*ModuleRawDataArrayPtrPtr) = &WorkerDataFrames;
        
        return true;
    } else {
        return false;
    }
}

FPSMoveWorker::~FPSMoveWorker()
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMove Destructor"));
    delete Thread;
    Thread = NULL;
}

bool FPSMoveWorker::Init()
{
    bool updated = UpdateMoveCount();
    return true;
}

uint32 FPSMoveWorker::Run()
{
    // I want the psmoves and psmove_tracker to be local variables in the thread.
    
    // Initialize an empty array of psmove controllers
    TArray<PSMove*> psmoves;
    
    // Initialize and configure the psmove_tracker
    PSMoveTracker *psmove_tracker = psmove_tracker_new(); // Unfortunately the API does not have a way to change the resolution and framerate.
    int tracker_width = 640;
    int tracker_height = 480;
    if (psmove_tracker)
    {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker initialized."));
        
        //Set exposure. TODO: Make this configurable.
        psmove_tracker_set_exposure(psmove_tracker, Exposure_MEDIUM);  //Exposure_LOW, Exposure_MEDIUM, Exposure_HIGH
        
        psmove_tracker_get_size(psmove_tracker, &tracker_width, &tracker_height);
        UE_LOG(LogPSMove, Log, TEXT("Camera Dimensions: %f x %f"), tracker_width, tracker_height);
    }
    else {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker failed to initialize."));
    }
    
    // TODO: Move this inside the loop to run if PSMoveCount changes.
    for (int i = 0; i < PSMoveCount; i++)
    {
        psmoves.Add(psmove_connect_by_id(i));
        assert(psmoves[i] != NULL);
        
        psmove_enable_orientation(psmoves[i], PSMove_True);
        assert(psmove_has_orientation(psmoves[i]));
        
        WorkerDataFrames[i].IsConnected = true;
        
        if (psmove_tracker)
        {
            while (psmove_tracker_enable(psmove_tracker, psmoves[i]) != Tracker_CALIBRATED); // Keep attempting to enable.
            
            //TODO: psmove_tracker_enable_with_color(psmove_tracker, psmoves[i], r, g, b)
            //TODO: psmove_tracker_get_color(psmove_tracker, psmoves[i], unisgned char &r, &g, &b);
            
            PSMove_Bool auto_update_leds = psmove_tracker_get_auto_update_leds(psmove_tracker, psmoves[i]);
            
            WorkerDataFrames[i].IsTracked = true;
        }
    }
    
    // Some variables we will need in the loop:
    float xpx, ypx, rpx;
    //float pr_conv = (M_PI/180.0) * (75.0/800.0); // 75 deg per 800 px

    //Initial wait before starting.
    FPlatformProcess::Sleep(0.03);

    while (StopTaskCounter.GetValue() == 0 && PSMoveCount > 0)
    {
        if (MoveCheckRequested && UpdateMoveCount())
        {
            // TODO: Fix psmoves, psmove_enable_orientation, psmove_tracker_enable
        }
        
        // Get positional data from tracker
        if (psmove_tracker)
        {
            // Renew the image on camera
            psmove_tracker_update_image(psmove_tracker); // Sometimes libusb crashes here.
            psmove_tracker_update(psmove_tracker, NULL); // Passing null (instead of m_moves[i]) updates all controllers.

            for (int i = 0; i < PSMoveCount; i++)
            {
                psmove_tracker_get_position(psmove_tracker, psmoves[i], &xpx, &ypx, &rpx);
                
                //zero x and y on camera's principal axis
                xpx = xpx - (tracker_width/2.0);  // Zero x on camera's principal axis
                ypx = (tracker_height/2.0) - ypx;  // Zero y on camera's principal axis. Note that ypx starts as pixels from top.
                
                // Convert ball radius (in pixels) to ball distance in cm.
                WorkerDataFrames[i].PosZ = psmove_tracker_distance_from_radius(psmove_tracker, rpx);  // Use Thomas' parametric fit to get depth in cm
                //zcm = 2.25 / sin( pr_conv * rpx ); //Use trigonometry to get depth in cm
                
                // We know sphere radius = 2.25 cm, which gives us pixel->cm conversion factor at this depth.
                WorkerDataFrames[i].PosX = xpx * 2.25 / rpx;
                WorkerDataFrames[i].PosY = ypx * 2.25 / rpx;

                /**
                 * psmoveapi's fusion classes do the following to transform these results into coordinates:
                 * projection = glm::perspectiveFov(fov, m_tracker_width, m_tracker_height, 0.1, 1000.0)
                 * viewport = glm::vec4(0., 0., m_tracker_width, m_tracker_height);
                 * modelview = glm::translate(glm::mat4(), glm::vec3(x, y, z)) * glm::mat4_cast(quaternion);
                 */
            }
        } else {
            FPlatformProcess::Sleep(0.001);
        }
        
        // Do bluetooth IO: Orientation, Buttons, Rumble
        for (int i = 0; i < PSMoveCount; i++)
        {
            //TODO: Is it necessary to keep polling until no frames are left?
            while (psmove_poll(psmoves[i]) > 0)
            {
                // Update the controller status (via bluetooth)
                psmove_poll(psmoves[i]);
                
                // Get the controller orientation (uses IMU).
                psmove_get_orientation(psmoves[i],
                                   &(WorkerDataFrames[i].OriW),
                                   &(WorkerDataFrames[i].OriX),
                                   &(WorkerDataFrames[i].OriY),
                                   &(WorkerDataFrames[i].OriZ));
                //I suppose it is possible that W,X,Y,Z do not get updated at the exact same moment.
                //If the rotator is calculated from w,x,y,z in between then it'll be wrong.
                //TODO: Use local floats then copy them all at once.
                
                // Get the controller button state
                WorkerDataFrames[i].Buttons = psmove_get_buttons(psmoves[i]);  // Bitwise; tells if each button is down.
                psmove_get_button_events(psmoves[i], &WorkerDataFrames[i].Pressed, &WorkerDataFrames[i].Released);  // i.e., state change
                
                // Get the controller trigger value (uint8; 0-255)
                WorkerDataFrames[i].TriggerValue = psmove_get_trigger(psmoves[i]);
                
                // Set the controller rumble (uint8; 0-255)
                psmove_set_rumble(psmoves[i], WorkerDataFrames[i].RumbleRequest);
            }
        }

        //Sleeping the thread seems to crash libusb.
        //FPlatformProcess::Sleep(0.005);

    }
    
    // Delete the controllers
    for (int i = 0; i<PSMoveCount; i++)
    {
        psmove_disconnect(psmoves[i]);
    }
    
    // Delete the tracker
    if (psmove_tracker)
    {
        psmove_tracker_free(psmove_tracker);
    }

    return 0;
}

void FPSMoveWorker::Stop()
{
    StopTaskCounter.Increment();
}

FPSMoveWorker* FPSMoveWorker::PSMoveWorkerInit(TArray<FPSMoveRawDataFrame>* &PSMoveRawDataArrayPtr)
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker::PSMoveWorkerInit"));
    if (!WorkerInstance && FPlatformProcess::SupportsMultithreading())
    {
        UE_LOG(LogPSMove, Log, TEXT("Creating new FPSMoveWorker instance."));
        WorkerInstance = new FPSMoveWorker(PSMoveRawDataArrayPtr);
    } else if (WorkerInstance) {
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