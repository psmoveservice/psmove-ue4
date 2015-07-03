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
#include "OVR_CAPI.h"

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
    PSMoveFusion *psmove_fusion = psmove_fusion_new(psmove_tracker, 1., 1000.);
    int tracker_width = 640;
    int tracker_height = 480;
    if (psmove_tracker)
    {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker initialized."));
        
        //Set exposure. TODO: Make this configurable.
        psmove_tracker_set_exposure(psmove_tracker, Exposure_MEDIUM);  //Exposure_LOW, Exposure_MEDIUM, Exposure_HIGH
        
        psmove_tracker_get_size(psmove_tracker, &tracker_width, &tracker_height);
        UE_LOG(LogPSMove, Log, TEXT("Camera Dimensions: %d x %d"), tracker_width, tracker_height);
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

    FTransform addedTransform;

    //TEMP Initialize OVR
    ovrBool ovrresult;
    ovrHmd HMD;
    ovrTrackingState dk2state;
    ovrresult = ovr_Initialize(0);
    ovrHmd_Create(0, &HMD);
    ovrresult = ovrHmd_ConfigureTracking(HMD,
        ovrTrackingCap_Orientation |
        ovrTrackingCap_MagYawCorrection |
        ovrTrackingCap_Position, 0);
    dk2state = ovrHmd_GetTrackingState(HMD, 0.0);

    // Convert dk2state.CameraPose to a 4x4 matrix, then to a float[16].
    float add_xf_arr[16] = { 0.5414, 0.5718, 0.6164, 0, -0.6008, 0.7760, -0.1921, 0, -0.5881, -0.2664, 0.7636, 0, 1.7955, -14.5762, -86.8742, 1.000 };
    /*
    addedTransform.SetComponents(
        FQuat(dk2state.CameraPose.Orientation.x, dk2state.CameraPose.Orientation.y, dk2state.CameraPose.Orientation.z, dk2state.CameraPose.Orientation.w),
        FVector(100.0*dk2state.CameraPose.Position.x, 100.0*dk2state.CameraPose.Position.y, 100.0*dk2state.CameraPose.Position.z),
        FVector(1.0, 1.0, 1.0));
    FMatrix addedMatrix = addedTransform.ToMatrixWithScale();
    int col_ix;
    float add_xf_arr[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1.000 };
    for (col_ix = 0; col_ix < 4; col_ix++)
    {
        add_xf_arr[col_ix * 4 + 0] = addedMatrix.GetColumn(col_ix).X;
        add_xf_arr[col_ix * 4 + 1] = addedMatrix.GetColumn(col_ix).Y;
        add_xf_arr[col_ix * 4 + 2] = addedMatrix.GetColumn(col_ix).Z;
    }
    */
    psmove_fusion_update_transform(psmove_fusion, add_xf_arr);
    printf("\t Fusion total transform:\n");
    float* xf_arr = psmove_fusion_get_transform_matrix(psmove_fusion);
    int el;
    for (el = 0; el < 16; el++)
    {
        UE_LOG(LogPSMove, Log, TEXT("xf_arr %f"), xf_arr[el]);
    }
    printf("\n");

    //Initial wait before starting.
    FPlatformProcess::Sleep(0.03);

    float xcm, ycm, zcm, oriw, orix, oriy, oriz;
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
                psmove_fusion_get_transformed_position(psmove_fusion, psmoves[i], &xcm, &ycm, &zcm);
                if (xcm && ycm && zcm && !isnan(xcm) && !isnan(ycm) && !isnan(zcm) && xcm==xcm && ycm==ycm && zcm==zcm)
                {
                    WorkerDataFrames[i].PosX = xcm;
                    WorkerDataFrames[i].PosY = ycm;
                    WorkerDataFrames[i].PosZ = zcm;
                }

                //UE_LOG(LogPSMove, Log, TEXT("X: %f, Y: %f, Z: %f"), xcm, ycm, zcm);
                if (WorkerDataFrames[i].ResetPoseRequest)
                {
                    psmove_tracker_reset_location(psmove_tracker, psmoves[i]);
                }
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
                    &oriw, &orix, &oriy, &oriz);
                WorkerDataFrames[i].OriW = oriw;
                WorkerDataFrames[i].OriX = orix;
                WorkerDataFrames[i].OriY = oriy;
                WorkerDataFrames[i].OriZ = oriz;
                //UE_LOG(LogPSMove, Log, TEXT("Ori w,x,y,z: %f, %f, %f, %f"), oriw, orix, oriy, oriz);
                
                // Get the controller button state
                WorkerDataFrames[i].Buttons = psmove_get_buttons(psmoves[i]);  // Bitwise; tells if each button is down.
                psmove_get_button_events(psmoves[i], &WorkerDataFrames[i].Pressed, &WorkerDataFrames[i].Released);  // i.e., state change
                
                // Get the controller trigger value (uint8; 0-255)
                WorkerDataFrames[i].TriggerValue = psmove_get_trigger(psmoves[i]);
                
                // Set the controller rumble (uint8; 0-255)
                psmove_set_rumble(psmoves[i], WorkerDataFrames[i].RumbleRequest);
            }
            if (WorkerDataFrames[i].ResetPoseRequest)
            {
                psmove_reset_orientation(psmoves[i]);
                WorkerDataFrames[i].ResetPoseRequest = false;  // Might not be thread safe?
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