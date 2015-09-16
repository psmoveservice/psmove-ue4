//  Created by Chadwick Boulay on 2015-02-20.
//  chadwick.boulay _at_ gmail.com
//  Many contributions by Brendan Walker brendan _at_ polyarcgames.com

// -- includes ----
#include "PSMovePrivatePCH.h"
#include "FPSMoveWorker.h"
#include "FPSMove.h"
#include <math.h>
#include <assert.h>

// -- constants ----
static const float CONTROLLER_COUNT_POLL_INTERVAL = 1000.f; // milliseconds

// -- private definitions ----

// TrackingContext contains references to the psmoveapi tracker and fusion objects, the controllers,
// and references to the shared (controller independent) data and the controller(s) data.
struct TrackingContext
{
    FPSMoveRawControllerWorkerData_TLS *WorkerControllerDataArray;
    
    PSMove* PSMoves[FPSMoveWorker::k_max_controllers];
    int32 PSMoveCount;
    uint32 LastMoveCountCheckTime;
    
    PSMoveTracker *PSMoveTracker;
    int TrackerWidth;
    int TrackerHeight;
    
    PSMoveFusion *PSMoveFusion;
    
    // Constructor
    TrackingContext(FPSMoveRawControllerWorkerData_TLS *controllerDataArray) :
        WorkerControllerDataArray(controllerDataArray)
    {
        Reset();
        
        // This timestamp is used to throttle how frequently we poll for controller count changes
        LastMoveCountCheckTime = FPlatformTime::Cycles();
    }
    
    void Reset()
    {
        memset(PSMoves, 0, sizeof(PSMoves));
        PSMoveCount = 0;
        LastMoveCountCheckTime = 0;
        PSMoveTracker = NULL;
        TrackerWidth = 0;
        TrackerHeight = 0;
        PSMoveFusion = NULL;
    }
};

// -- prototypes ----
static bool TrackingContextSetup(TrackingContext *context);                                 //
static bool TrackingContextIsSetup(TrackingContext *context);                               //
static bool TrackingContextUpdateControllerConnections(TrackingContext *context);           // Add or remove PSMove controllers.
static void TrackingContextTeardown(TrackingContext *context);                              //
static void ControllerUpdatePositions(PSMoveTracker *psmove_tracker,                        // 
                                      PSMoveFusion *psmove_fusion,
                                      PSMove *psmove,
                                      FPSMoveRawControllerData_Base *controllerData);
static void ControllerUpdateOrientations(PSMove *psmove,                                    //
                                         FPSMoveRawControllerData_Base *controllerData);
static void ControllerUpdateButtonState(PSMove *psmove,                                     //
                                        FPSMoveRawControllerData_Base *controllerData);

// -- Worker Thread --
FPSMoveWorker* FPSMoveWorker::WorkerInstance = NULL;

// The worker spawns the thread that communicates with the PSMoveAPI in parallel to the game thread.
FPSMoveWorker::FPSMoveWorker() :
    AcquiredContextCounter(0),
    StopTaskCounter(0)
{
    // Setup the controller data entries
    for (int32 controller_index = 0; controller_index < FPSMoveWorker::k_max_controllers; ++controller_index)
    {
        // Make sure the controller entries are initialized
        WorkerControllerDataArray[controller_index].Clear();
        WorkerControllerDataArray_Concurrent[controller_index].Clear();

        // Bind the controller worker concurrent data to its corresponding thread-local container
        WorkerControllerDataArray[controller_index].ConcurrentData = &WorkerControllerDataArray_Concurrent[controller_index];
    }
    
    // This Inits and Runs the thread.
    Thread = FRunnableThread::Create(this, TEXT("FPSMoveWorker"), 0, TPri_AboveNormal);
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

        // The worker thread will create a tracker if one isn't active at this moment
        AcquiredContextCounter.Increment();

        success = true;
    }

    return success;
}

void FPSMoveWorker::ReleasePSMove(FPSMoveDataContext *DataContext)
{
    if (DataContext->PSMoveID != -1)
    {
        DataContext->Clear();
        
        // The worker thread will tear-down the tracker
        assert(AcquiredContextCounter.GetValue() > 0);
        AcquiredContextCounter.Decrement();
    }
}

uint32 FPSMoveWorker::Run()
{
    // Maintains the following psmove state on the stack
    // * psmove tracking state
    // * psmove fusion state
    // * psmove controller state
    // Tracking state is only initialized when we have a non-zero number of tracking contexts
    TrackingContext Context(WorkerControllerDataArray);
    
    if (!psmove_init(PSMOVE_CURRENT_VERSION))
    {
        UE_LOG(LogPSMove, Error, TEXT("PS Move API init failed (wrong version?)"));
        return -1;
    }
    
    while (StopTaskCounter.GetValue() == 0)
    {
        // If there are component contexts active, make sure the tracking context is setup
        if (AcquiredContextCounter.GetValue() > 0 && !TrackingContextIsSetup(&Context))
        {
            TrackingContextSetup(&Context);
        }
        // If there are no component contexts active, make sure the tracking context is torn-down
        else if (AcquiredContextCounter.GetValue() <= 0 && TrackingContextIsSetup(&Context))
        {
            TrackingContextTeardown(&Context);
        }
        
        // Update controller state while tracking is active
        if (TrackingContextIsSetup(&Context))
        {
            QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_RunLoop)
                       
            // Setup or tear down controller connections based on the number of active controllers
            TrackingContextUpdateControllerConnections(&Context);
            
            // Update the raw positions of the controllers
            {
                QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_UpdateImage)
                
                // Renew the image on camera
                psmove_tracker_update_image(Context.PSMoveTracker); // Sometimes libusb crashes here.
            }
            
            // Update the raw positions on the local controller data
            {
                QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_Position)
                
                for (int psmove_id = 0; psmove_id < Context.PSMoveCount; psmove_id++)
                {
                    FPSMoveRawControllerWorkerData_TLS &localControllerData = WorkerControllerDataArray[psmove_id];
                                
                    ControllerUpdatePositions(Context.PSMoveTracker,
                                              Context.PSMoveFusion,
                                              Context.PSMoves[psmove_id],
                                              &localControllerData);
                }
            }
            
            // Do bluetooth IO: Orientation, Buttons, Rumble
            {
                QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_Polling)
                
                for (int psmove_id = 0; psmove_id < Context.PSMoveCount; psmove_id++)
                {
                    //TODO: Is it necessary to keep polling until no frames are left?
                    while (psmove_poll(Context.PSMoves[psmove_id]) > 0)
                    {
                        FPSMoveRawControllerWorkerData_TLS &localControllerData = WorkerControllerDataArray[psmove_id];
                        
                        // Update the controller status (via bluetooth)
                        psmove_poll(Context.PSMoves[psmove_id]);  // Necessary to poll yet again?
                        
                        // Store the controller orientation
                        ControllerUpdateOrientations(Context.PSMoves[psmove_id], &localControllerData);
                        
                        // Store the button state
                        ControllerUpdateButtonState(Context.PSMoves[psmove_id], &localControllerData);

                        // Now read in requested changes from Component. e.g., RumbleRequest, ResetPoseRequest, CycleColourRequest
                        localControllerData.WorkerRead();
                        
                        // Set the controller rumble (uint8; 0-255)
                        psmove_set_rumble(Context.PSMoves[psmove_id], localControllerData.RumbleRequest);
                        
                        // See if the reset pose request has been posted by the component.
                        // It is not recommended to use this. We will soon expose a psmove_reset_yaw function that should be used instead.
                        // Until then, use the local yaw reset in the psmove component.
                        if (localControllerData.ResetPoseRequest)
                        {
                            UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker:: RESET POSE"));
                            
                            psmove_reset_orientation(Context.PSMoves[psmove_id]);
                            psmove_tracker_reset_location(Context.PSMoveTracker, Context.PSMoves[psmove_id]);
                            
                            // Clear the request flag now that we've handled the request
                            localControllerData.ResetPoseRequest = false;
                        }

                        if (localControllerData.CycleColourRequest)
                        {
                            UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker:: CYCLE COLOUR"));
                            psmove_tracker_cycle_color(Context.PSMoveTracker, Context.PSMoves[psmove_id]);
                            localControllerData.CycleColourRequest = false;
                        }

                        // Publish the worker data to the component. e.g., Position, Orientation, Buttons
                        // This also publishes updated ResetPoseRequest and CycleColourRequest.
                        localControllerData.WorkerPost();
                    }
                }
            }
            
            // Update the clock. This is left in here from old code in case we need a clock in the future (e.g., prediction)
            {
                QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_Filtering)
                
                for (int psmove_id = 0; psmove_id < Context.PSMoveCount; psmove_id++)
                {
                    FPSMoveRawControllerWorkerData_TLS &localControllerData = WorkerControllerDataArray[psmove_id];
                    
                    // Refresh the clock for the controller.
                    // Was needed for filter. Still needed?
                    localControllerData.Clock.Update();
                }
            }
        }
        else
        {
            // Wait again before trying to activate the camera
            FPlatformProcess::Sleep(1);
        }
    }
    
    if (!TrackingContextIsSetup(&Context))
    {
        TrackingContextTeardown(&Context);
    }

    // Free any dependent APIs
    psmove_shutdown();

    return 0;
}

void FPSMoveWorker::Stop()
{
    StopTaskCounter.Increment();
}

FPSMoveWorker* FPSMoveWorker::InitializeSingleton()
{
    UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker::InitializeSingleton"));
    if (!WorkerInstance && FPlatformProcess::SupportsMultithreading())
    {
        UE_LOG(LogPSMove, Log, TEXT("Creating new FPSMoveWorker instance."));
        WorkerInstance = new FPSMoveWorker();
    } else if (WorkerInstance) {
        UE_LOG(LogPSMove, Log, TEXT("FPSMoveWorker already instanced."));
    }
    return WorkerInstance;
}

void FPSMoveWorker::ShutdownSingleton()
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

//-- private methods ----
static bool TrackingContextSetup(TrackingContext *context)
{
    bool success = true;
    
    // Clear out the tracking state
    // Reset the shared worker data
    context->Reset();
    
    UE_LOG(LogPSMove, Log, TEXT("Setting up PSMove Tracking Context"));
    
    // Initialize and configure the psmove_tracker.
    {
        PSMoveTrackerSettings settings;
        psmove_tracker_settings_set_default(&settings);
        settings.color_mapping_max_age = 0; // Don't used cached color mapping file
        settings.exposure_mode = Exposure_LOW;
        settings.use_fitEllipse = 1;
        settings.camera_mirror = PSMove_True;
        context->PSMoveTracker = psmove_tracker_new_with_settings(&settings);
    }
    
    if (context->PSMoveTracker != NULL)
    {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker initialized."));
        
        PSMoveTrackerSmoothingSettings smoothing_settings;
        psmove_tracker_get_smoothing_settings(context->PSMoveTracker, &smoothing_settings);
        smoothing_settings.filter_do_2d_r = 0;
        smoothing_settings.filter_do_2d_xy = 0;
        smoothing_settings.filter_3d_type = Smoothing_LowPass;
        psmove_tracker_set_smoothing_settings(context->PSMoveTracker, &smoothing_settings);

        psmove_tracker_get_size(context->PSMoveTracker, &context->TrackerWidth, &context->TrackerHeight);
        UE_LOG(LogPSMove, Log, TEXT("Camera Dimensions: %d x %d"), context->TrackerWidth, context->TrackerHeight);
    }
    else
    {
        UE_LOG(LogPSMove, Error, TEXT("PSMove tracker failed to initialize."));
        success = false;
    }
    
    // Initialize fusion api if the tracker started
    if (success)
    {
        context->PSMoveFusion = psmove_fusion_new(context->PSMoveTracker, 1.0f, 1000.0f);
        
        if (context->PSMoveFusion != NULL)
        {
            UE_LOG(LogPSMove, Log, TEXT("PSMove fusion initialized."));
        }
        else
        {
            UE_LOG(LogPSMove, Error, TEXT("PSMove failed to initialize."));
            success = false;
        }
    }
    
    if (!success)
    {
        TrackingContextTeardown(context);
    }
    
    return success;
}

static bool TrackingContextIsSetup(TrackingContext *context)
{
    return context->PSMoveTracker != NULL && context->PSMoveFusion != NULL;
}

static bool TrackingContextUpdateControllerConnections(TrackingContext *context)
{
    bool controllerCountChanged = false;
    uint32 currentTime = FPlatformTime::Cycles();
    float millisecondsSinceCheck = FPlatformTime::ToMilliseconds(currentTime - context->LastMoveCountCheckTime);
    
    assert(TrackingContextIsSetup(context));
    
    if (millisecondsSinceCheck >= CONTROLLER_COUNT_POLL_INTERVAL)
    {
        // Update the number
        int newcount = psmove_count_connected();
        
        if (context->PSMoveCount != newcount)
        {
            UE_LOG(LogPSMove, Log, TEXT("PSMove Controllers count changed: %d -> %d."), context->PSMoveCount, newcount);
            
            context->PSMoveCount = newcount;
            controllerCountChanged = true;
        }
        
        // Refresh the connection and tracking state of every controller entry
        for (int psmove_id = 0; psmove_id < FPSMoveWorker::k_max_controllers; psmove_id++)
        {
            if (psmove_id < context->PSMoveCount)
            {
                if (context->PSMoves[psmove_id] == NULL)
                {
                    // The controller should be connected
                    context->PSMoves[psmove_id] = psmove_connect_by_id(psmove_id);
                    
                    if (context->PSMoves[psmove_id] != NULL)
                    {
                        psmove_enable_orientation(context->PSMoves[psmove_id], PSMove_True);
                        assert(psmove_has_orientation(context->PSMoves[psmove_id]));
                        
                        // Don't apply any transform to the sensor data,
                        // We'll handle that in the PSMoveComponent.
                        psmove_set_sensor_data_transform(context->PSMoves[psmove_id], k_psmove_sensor_transform_identity);

                        context->WorkerControllerDataArray[psmove_id].IsConnected = true;
                    }
                    else
                    {
                        context->WorkerControllerDataArray[psmove_id].IsConnected = false;
                        UE_LOG(LogPSMove, Error, TEXT("Failed to connect to PSMove controller %d"), psmove_id);
                    }
                }
                
                if (context->PSMoves[psmove_id] != NULL && context->WorkerControllerDataArray[psmove_id].IsEnabled == false)
                {
                    // The controller is connected, but not tracking yet
                    // Enable tracking for this controller with next available colour.
                    if (psmove_tracker_enable(context->PSMoveTracker, context->PSMoves[psmove_id]) == Tracker_CALIBRATED)
                    {
                        context->WorkerControllerDataArray[psmove_id].Clock.Initialize();
                        context->WorkerControllerDataArray[psmove_id].IsEnabled = true;
                    }
                    else
                    {
                        UE_LOG(LogPSMove, Error, TEXT("Failed to enable tracking for PSMove controller %d"), psmove_id);
                    }
                }
            }
            else
            {// The controller should no longer be tracked
                if (context->PSMoves[psmove_id] != NULL)
                {
                    psmove_disconnect(context->PSMoves[psmove_id]);
                    context->PSMoves[psmove_id] = NULL;
                    context->WorkerControllerDataArray[psmove_id].IsEnabled = false;
                    context->WorkerControllerDataArray[psmove_id].IsConnected = false;
                }
            }
        }
        
        // Remember the last time we polled the move count
        context->LastMoveCountCheckTime = currentTime;
    }
    
    return controllerCountChanged;
}

static void TrackingContextTeardown(TrackingContext *context)
{
    UE_LOG(LogPSMove, Log, TEXT("Tearing down PSMove Tracking Context"));
    
    // Delete the controllers
    for (int psmove_id = 0; psmove_id < FPSMoveWorker::k_max_controllers; psmove_id++)
    {
        if (context->PSMoves[psmove_id] != NULL)
        {
            UE_LOG(LogPSMove, Log, TEXT("Disconnecting PSMove controller %d"), psmove_id);
            context->WorkerControllerDataArray[psmove_id].IsConnected = false;
            context->WorkerControllerDataArray[psmove_id].IsEnabled = false;
            psmove_disconnect(context->PSMoves[psmove_id]);
        }
    }
    
    // Delete the tracking fusion state
    if (context->PSMoveFusion != NULL)
    {
        UE_LOG(LogPSMove, Log, TEXT("PSMove fusion disposed"));
        psmove_fusion_free(context->PSMoveFusion);
    }
    
    // Delete the tracker state
    if (context->PSMoveTracker != NULL)
    {
        UE_LOG(LogPSMove, Log, TEXT("PSMove tracker disposed"));
        psmove_tracker_free(context->PSMoveTracker);
    }
    
    context->Reset();
}

static void ControllerUpdatePositions(PSMoveTracker *psmove_tracker,
                                      PSMoveFusion *psmove_fusion,
                                      PSMove *psmove,
                                      FPSMoveRawControllerData_Base *controllerData)
{
    // Find the sphere position in the camera
    int found_sphere = 0;
    {
        QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_TrackerUpdate)
        found_sphere = psmove_tracker_update(psmove_tracker, psmove);
    }
    
    enum PSMoveTracker_Status curr_status =
        psmove_tracker_get_status(psmove_tracker, psmove);
    
    // Can we actually see the controller this frame?
    controllerData->IsTracking = curr_status == Tracker_TRACKING;

    // Update the position of the controller
    if (controllerData->IsTracking)
    {
        QUICK_SCOPE_CYCLE_COUNTER(Stat_FPSMoveWorker_TrackerFusion)
        
        float xcm, ycm, zcm;
        psmove_fusion_get_transformed_location(psmove_fusion, psmove, &xcm, &ycm, &zcm);
        
        // [Store the controller position]
        // Remember the position the ps move controller in either its native space
        // or in a transformed space if a transform file existed.
        controllerData->PSMovePosition = FVector(xcm, ycm, zcm);
    }
}

static void ControllerUpdateOrientations(PSMove *psmove,
                                         FPSMoveRawControllerData_Base *controllerData)
{
    float oriw, orix, oriy, oriz;
    
    // Get the controller orientation (uses IMU).
    psmove_get_orientation(psmove, &oriw, &orix, &oriy, &oriz);
    
    controllerData->PSMoveOrientation = FQuat(orix, oriy, oriz, oriw);
    
    /*
    controllerData->WorldRelativeFilteredOrientation =
    // Convert from the controller coordinate system to Unreal's coordinate system where
    FQuat(oriz, -orix, -oriy, oriw);
    */
}

static void ControllerUpdateButtonState(PSMove *psmove,
                                        FPSMoveRawControllerData_Base *controllerData)
{
    // Get the controller button state
    controllerData->Buttons = psmove_get_buttons(psmove);  // Bitwise; tells if each button is down.
    
    // Get the controller trigger value (uint8; 0-255)
    controllerData->TriggerValue = psmove_get_trigger(psmove);
}
