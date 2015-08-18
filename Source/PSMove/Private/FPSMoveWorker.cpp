//
//  FPSMoveWorker.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-20.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//

// -- includes ----
#include "PSMovePrivatePCH.h"
#include "FPSMoveWorker.h"
#include "FPSMove.h"
#include <math.h>
#include <assert.h>

// -- constants ----
static const float CONTROLLER_COUNT_POLL_INTERVAL = 1000.f; // milliseconds

// -- Worker Thread --
FPSMoveWorker* FPSMoveWorker::WorkerInstance = NULL;

FPSMoveWorker::FPSMoveWorker()
    : StopTaskCounter(0)
	, PSMoveCount(0)
	, LastMoveCountCheckTime(0)
{
	// Setup the controller data entries
	for (int32 controller_index = 0; controller_index < FPSMoveWorker::k_max_controllers; ++controller_index)
	{
		// Make sure the controller entries are initialize
		WorkerControllerDataArray[controller_index].Clear();
		WorkerControllerDataArray_Concurrent[controller_index].Clear();

		// Bind the controller worker concurrent data to it's corresponding thread-local container
		WorkerControllerDataArray[controller_index].ConcurrentData = &WorkerControllerDataArray_Concurrent[controller_index];
	}
    
    // This Inits and Runs the thread.
    Thread = FRunnableThread::Create(this, TEXT("FPSMoveWorker"), 0, TPri_Normal);
}

bool FPSMoveWorker::UpdateControllerConnections(
	PSMoveTracker *Tracker,
	PSMove **PSMoves)
{
	bool controllerCountChanged = false;
	uint32 currentTime = FPlatformTime::Cycles();
	float millisecondsSinceCheck = FPlatformTime::ToMilliseconds(currentTime - this->LastMoveCountCheckTime);

	if (millisecondsSinceCheck >= CONTROLLER_COUNT_POLL_INTERVAL)
	{
		// Update the number 
		int newcount = psmove_count_connected();

		if (this->PSMoveCount != newcount)
		{
			UE_LOG(LogPSMove, Log, TEXT("PSMove Controllers count changed: %d -> %d."), this->PSMoveCount, newcount);

			this->PSMoveCount = newcount;
			controllerCountChanged = true;
		}

		// Refresh the connection and tracking state of every controller entry
		for (int psmove_id = 0; psmove_id < FPSMoveWorker::k_max_controllers; psmove_id++)
		{
			if (psmove_id < this->PSMoveCount)
			{
				if (PSMoves[psmove_id] == NULL)
				{
					// The controller should be connected
					PSMoves[psmove_id] = psmove_connect_by_id(psmove_id);

					if (PSMoves[psmove_id] != NULL)
					{
						psmove_enable_orientation(PSMoves[psmove_id], PSMove_True);
						assert(psmove_has_orientation(PSMoves[psmove_id]));

						this->WorkerControllerDataArray[psmove_id].IsConnected = true;
					}
					else
					{
						this->WorkerControllerDataArray[psmove_id].IsConnected = false;
						UE_LOG(LogPSMove, Error, TEXT("Failed to connect to PSMove controller %d"), psmove_id);
					}
				}

				if (PSMoves[psmove_id] != NULL && this->WorkerControllerDataArray[psmove_id].IsCalibrated == false)
				{
					PSMoveTracker_Status tracking_status = psmove_tracker_enable(Tracker, PSMoves[psmove_id]);
					psmove_tracker_set_auto_update_leds(Tracker, PSMoves[psmove_id], PSMove_True);
					
					if (tracking_status == Tracker_CALIBRATED)
					{
						this->WorkerControllerDataArray[psmove_id].IsCalibrated = true;
					}
					else
					{
						UE_LOG(LogPSMove, Error, TEXT("Failed to enable tracking for PSMove controller %d (result status: %d)"), psmove_id, (int32)tracking_status);
					}
				}
			}
			else
			{
				// The controller should no longer be tracked
				if (PSMoves[psmove_id] != NULL)
				{
					psmove_disconnect(PSMoves[psmove_id]);
					PSMoves[psmove_id] = NULL;
					this->WorkerControllerDataArray[psmove_id].IsTracked = false;
					this->WorkerControllerDataArray[psmove_id].IsCalibrated = false;
					this->WorkerControllerDataArray[psmove_id].IsConnected = false;
				}
			}
		}

		// Remember the last time we polled the move count
		this->LastMoveCountCheckTime = currentTime;
	}

	return controllerCountChanged;
}

FPSMoveWorker::~FPSMoveWorker()
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMove Destructor"));
    delete Thread;
    Thread = NULL;
}

bool FPSMoveWorker::Init()
{
    return true;
}

bool FPSMoveWorker::AcquirePSMove(
	int32 PSMoveID,
	FPSMoveDataContext *DataContext)
{
	bool success = false;

	if (PSMoveID >= 0 && PSMoveID < FPSMoveWorker::k_max_controllers)
	{
		// Remember which PSMove the data context is assigned to
		DataContext->Clear();
		DataContext->PSMoveID = PSMoveID;

		// Bind the data context to the concurrent data for the requested controller
		// This doesn't mean  that the controller is active, just that a component
		// is now watching this block of data.
		// Also this is thread safe because were not actually looking at the concurrent data
		// at this point, just assigning a pointer to the concurrent data.
		DataContext->RawControllerData.ConcurrentData = &WorkerControllerDataArray_Concurrent[PSMoveID];

		success = true;
	}

	return success;
}

uint32 FPSMoveWorker::Run()
{
    // I want the psmoves and psmove_tracker to be local variables in the thread.
    
    // Initialize an empty array of psmove controllers
	PSMove* psmoves[FPSMoveWorker::k_max_controllers];
	memset(psmoves, 0, sizeof(psmoves));

    // Initialize and configure the psmove_tracker
    PSMoveTracker *psmove_tracker = psmove_tracker_new(); // Unfortunately the API does not have a way to change the resolution and framerate.
    PSMoveFusion *psmove_fusion = psmove_fusion_new(psmove_tracker, 1., 1000.);
    int tracker_width = 640;
    int tracker_height = 480;
    if (psmove_tracker)
    {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker initialized."));
        
        //Set exposure. TODO: Expose this to component.
        psmove_tracker_set_exposure(psmove_tracker, Exposure_MEDIUM);  //Exposure_LOW, Exposure_MEDIUM, Exposure_HIGH
        psmove_tracker_set_smoothing(psmove_tracker, 0, 1);
		psmove_tracker_set_mirror(psmove_tracker, PSMove_True);
        
        psmove_tracker_get_size(psmove_tracker, &tracker_width, &tracker_height);
        UE_LOG(LogPSMove, Log, TEXT("Camera Dimensions: %d x %d"), tracker_width, tracker_height);
    }
    else {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker failed to initialize."));
    }
    
    //Initial wait before starting.
    FPlatformProcess::Sleep(0.03);

    float xcm, ycm, zcm, oriw, orix, oriy, oriz;
    while (StopTaskCounter.GetValue() == 0)
    {        
        // Get positional data from tracker
        if (psmove_tracker)
        {            
			// Setup or tear down controller connections based on the number of active controllers
			UpdateControllerConnections(psmove_tracker, psmoves);

			// Renew the image on camera
			if (PSMoveCount > 0)
			{
				psmove_tracker_update_image(psmove_tracker); // Sometimes libusb crashes here.
				psmove_tracker_update_cbb(psmove_tracker, NULL); // Passing null (instead of m_moves[i]) updates all controllers.
			}
		}
		else {
			FPlatformProcess::Sleep(0.001);
		}

		for (int i = 0; i < PSMoveCount; i++)
		{
			FPSMoveRawControllerData_TLS &localControllerData = WorkerControllerDataArray[i];

			//--------------
			// Read the published data from the component
			//--------------
			localControllerData.WorkerRead();

			// Get positional data from tracker
			if (psmove_tracker)
            {
                localControllerData.IsTracked = psmove_tracker_get_status(psmove_tracker, psmoves[i]) == Tracker_TRACKING;

                psmove_fusion_get_transformed_location(psmove_fusion, psmoves[i], &xcm, &ycm, &zcm);

                if (localControllerData.IsTracked &&
                    xcm && ycm && zcm &&
                    !isnan(xcm) && !isnan(ycm) && !isnan(zcm) &&
                    xcm == xcm && ycm == ycm && zcm == zcm)
                {
                    localControllerData.PosX = xcm;
                    localControllerData.PosY = ycm;
                    localControllerData.PosZ = zcm;
                }
                else {
                    localControllerData.IsTracked = false;
                }

                //UE_LOG(LogPSMove, Log, TEXT("X: %f, Y: %f, Z: %f"), xcm, ycm, zcm);
                if (localControllerData.ResetPoseRequest && localControllerData.IsTracked)
                {
                    psmove_tracker_reset_location(psmove_tracker, psmoves[i]);
                }

                // If we are to change the tracked colour.
                if (localControllerData.UpdateLedRequest)
                {
                    // Stop tracking the controller with the existing color
                    psmove_tracker_disable(psmove_tracker, psmoves[i]);
                    localControllerData.IsTracked = false;
                    localControllerData.IsCalibrated = false;

                    psmove_tracker_set_dimming(psmove_tracker, 0.0);  // Set dimming to 0 to trigger blinking calibration.
                    psmove_set_leds(psmoves[i], 0, 0, 0);  // Turn off the LED to make sure it isn't trackable until new colour set.
                    psmove_update_leds(psmoves[i]);
                    FColor newFColor = localControllerData.LedColourRequest.Quantize();

                    if (psmove_tracker_enable_with_color(psmove_tracker, psmoves[i], newFColor.R, newFColor.G, newFColor.B) == Tracker_CALIBRATED)
                    {
                        this->WorkerControllerDataArray[i].IsCalibrated = true;
                    }
                    else
                    {
                        UE_LOG(LogPSMove, Error, TEXT("Failed to change tracking color for PSMove controller %d"), i);
                    }

                    localControllerData.LedColourWasUpdated = true;
                }
                else
                {
                    localControllerData.LedColourWasUpdated = false;
                }

			}
			else {
				FPlatformProcess::Sleep(0.001);
			}

			// Do bluetooth IO: Orientation, Buttons, Rumble
			
            //TODO: Is it necessary to keep polling until no frames are left?
            while (psmove_poll(psmoves[i]) > 0)
            {
                // Update the controller status (via bluetooth)
                psmove_poll(psmoves[i]);

                // Get the controller orientation (uses IMU).
                psmove_get_orientation(psmoves[i],
                    &oriw, &orix, &oriy, &oriz);
                localControllerData.OriW = oriw;
                localControllerData.OriX = orix;
                localControllerData.OriY = oriy;
                localControllerData.OriZ = oriz;
                //UE_LOG(LogPSMove, Log, TEXT("Ori w,x,y,z: %f, %f, %f, %f"), oriw, orix, oriy, oriz);

                // Get the controller button state
                localControllerData.Buttons = psmove_get_buttons(psmoves[i]);  // Bitwise; tells if each button is down.
                psmove_get_button_events(psmoves[i], &localControllerData.Pressed, &localControllerData.Released);  // i.e., state change

                // Get the controller trigger value (uint8; 0-255)
                localControllerData.TriggerValue = psmove_get_trigger(psmoves[i]);

                // Set the controller rumble (uint8; 0-255)
                psmove_set_rumble(psmoves[i], localControllerData.RumbleRequest);
            }

            if (localControllerData.ResetPoseRequest)
            {
                psmove_reset_orientation(psmoves[i]);
                localControllerData.PoseWasReset = true;
            }
            else {
                localControllerData.PoseWasReset = false;
            }


			//--------------
			// Publish the updated worker data to the component
			//--------------
			localControllerData.WorkerPost();
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

FPSMoveWorker* FPSMoveWorker::PSMoveWorkerInit()
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker::PSMoveWorkerInit"));
    if (!WorkerInstance && FPlatformProcess::SupportsMultithreading())
    {
        UE_LOG(LogPSMove, Log, TEXT("Creating new FPSMoveWorker instance."));
        WorkerInstance = new FPSMoveWorker();
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