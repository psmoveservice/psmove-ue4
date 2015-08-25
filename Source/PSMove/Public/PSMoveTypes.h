#pragma once

#include "EnumAsByte.h"
#include "FPSMoveClock.h"

//---------------------------------------------------
// Delegate types
//---------------------------------------------------
#include "PSMoveTypes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveDataUpdatedDelegate, FVector, Position, FRotator, Rotation, bool, IsTracking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveButtonStateDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveTriggerButtonDelegate, uint8, Value);

//---------------------------------------------------
// Structures
//---------------------------------------------------

// Data needed by the worker common to all active move controllers.
// Currently this is only used for communicating PSMoveCount, but it could be
// used for other things common to all controllers (e.g., tracker settings).
USTRUCT()
struct FPSMoveRawSharedData_Base
{
    GENERATED_USTRUCT_BODY();
    
    UPROPERTY()
    uint8 PSMoveCount;
    
    UPROPERTY()
    bool ComponentHasReadWorkerData;  //Unused?
    
    FPSMoveRawSharedData_Base()
    {
        Clear();
    }
    
    inline void Clear()
    {
        PSMoveCount = 0;
        ComponentHasReadWorkerData = true; //Unused?
    }
};

// A lockable version of the data frame to be used by the worker thread.
USTRUCT()
struct FPSMoveRawSharedData_Concurrent : public FPSMoveRawSharedData_Base
{
    GENERATED_USTRUCT_BODY();
    
    // Used to make sure we're accessing the data in a thread safe way
    FCriticalSection Lock;
    
    FPSMoveRawSharedData_Concurrent() :
        Lock()
    {
        Clear();
    }
    
    inline void Clear()
    {
        FPSMoveRawSharedData_Base::Clear();
    }
};

// A version of the data shared data frame to be used by the component.
USTRUCT()
struct FPSMoveRawSharedData_TLS : public FPSMoveRawSharedData_Base
{
    GENERATED_USTRUCT_BODY();
    
    // This is a pointer to the worker's copy of the data.
    FPSMoveRawSharedData_Concurrent *ConcurrentData;
    
    // Flag to denote the worker has modified its TLS data
    bool WorkerHasModifiedTLSData;
    
    FPSMoveRawSharedData_TLS()
    {
        ConcurrentData = nullptr;
        Reset();
    }
    
    void Reset()
    {
        FPSMoveRawSharedData_Base::Clear();
        WorkerHasModifiedTLSData= false;
    }
    
    bool IsValid() const
    {
        return ConcurrentData != nullptr;
    }
    
    void MarkDirty()
    {
        this->WorkerHasModifiedTLSData = true;
    }
    
    void ComponentRead()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawSharedData_TLS::ComponentRead"), 500);
        
        {
            FScopeLock scopeLock(&ConcurrentData->Lock);
            
            // Read the worker thread's data into the component thread's data
            this->PSMoveCount = ConcurrentData->PSMoveCount;
            
            // We've read the data the worker posted, so it's no longer dirty
            this->ComponentHasReadWorkerData = true;
            ConcurrentData->ComponentHasReadWorkerData = true;
        }
    }

    void ComponentPost()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawSharedData_TLS::ComponentPost"), 500);

        {
            FScopeLock scopeLock(&ConcurrentData->Lock);
        }
    }
    
    void WorkerRead()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawSharedData_TLS::WorkerRead"), 500);
        
        {
            FScopeLock scopeLock(&ConcurrentData->Lock);
            
            // Read the component thread's data into the worker thread's data
            this->ComponentHasReadWorkerData = ConcurrentData->ComponentHasReadWorkerData;
        }
    }
    
    void WorkerPost()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawSharedData_TLS::WorkerPost"), 500);
        
        if (WorkerHasModifiedTLSData)
        {
            FScopeLock scopeLock(&ConcurrentData->Lock);
            
            // The component has new data to read from the worker thread
            ConcurrentData->ComponentHasReadWorkerData = false;
            this->ComponentHasReadWorkerData = false;
            
            // The worker threads TLS data is no longer dirty
            WorkerHasModifiedTLSData = false;
        }
    }
};

// The controller-specific data frame
USTRUCT()
struct FPSMoveRawControllerData_Base
{
    GENERATED_USTRUCT_BODY();
    
    // -------
    // Data items that typically get filled by the Worker thread
    // then are made available to the consuming game object (i.e. component)
    
    UPROPERTY()
    FVector PSMovePosition;
    
    UPROPERTY()
    FQuat PSMoveOrientation;
    
    UPROPERTY()
    uint32 Buttons;
    
    UPROPERTY()
    uint32 Pressed;
    
    UPROPERTY()
    uint32 Released;
    
    UPROPERTY()
    uint8 TriggerValue;

    UPROPERTY()
    bool IsConnected;           // Whether or not the controller is connected

    UPROPERTY()
    bool IsTracking;            // Whether or not the tracker saw the controller on the latest frame.
    
    UPROPERTY()
    bool IsEnabled;             // Whether or not the tracker is tracking this specific controller
    
    UPROPERTY()
    bool IsCalibrated;          // I don't know

    // -------
    // Data items that typically get filled by the game object (i.e., PSMoveComponent)
    // then are passed to the worker thread.

    UPROPERTY()
    uint8 RumbleRequest;            // Please make the controller rumbled

    bool ResetPoseRequest;          // Please use the current pose as zero-pose.

    bool CycleColourRequest;        // Please change to the next available colour.

    // -----------
    // Constructor
    FPSMoveRawControllerData_Base()
    {
        Clear();
    }

    void Clear()
    {
        PSMovePosition = FVector::ZeroVector;
        PSMoveOrientation = FQuat::Identity;
        Buttons = 0;
        Pressed = 0;
        Released = 0;
        TriggerValue = 0;
        RumbleRequest = 0;
        ResetPoseRequest = false;
        CycleColourRequest = false;
        IsConnected = false;
        IsCalibrated = false;
        IsTracking = false;
        IsEnabled = false;
    }
};

//A version of the data frame to be used by the worker thread.
USTRUCT()
struct FPSMoveRawControllerData_Concurrent : public FPSMoveRawControllerData_Base
{
    GENERATED_USTRUCT_BODY();

    // Used to make sure we're accessing the data in a thread safe way
    FCriticalSection Lock;

    FPSMoveRawControllerData_Concurrent() :
        Lock()
    {
        Clear();
    }

    inline void Clear()
    {
        FPSMoveRawControllerData_Base::Clear();
    }
};

// A version of the data frame that will be used by the controller.
// It contains a thread-safe copy of the data frame to be accessed by the worker
// and some methods for keeping the worker data and controller data sync'd.
USTRUCT()
struct FPSMoveRawControllerData_TLS : public FPSMoveRawControllerData_Base
{
    GENERATED_USTRUCT_BODY();

    // This is a pointer to a copy of the data that wil
    FPSMoveRawControllerData_Concurrent *ConcurrentData;

    FPSMoveRawControllerData_TLS()
    {
        Clear();
    }

    void Clear()
    {
        FPSMoveRawControllerData_Base::Clear();
        ConcurrentData = nullptr;
    }

    bool IsValid() const
    {
        return ConcurrentData != nullptr;
    }

    void ComponentRead()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawControllerData_TLS::ComponentPostAndRead"), 500);
        {
            FScopeLock scopeLock(&ConcurrentData->Lock);

            // Read the worker thread's data into the component thread's data
            this->PSMovePosition = ConcurrentData->PSMovePosition;
            this->PSMoveOrientation = ConcurrentData->PSMoveOrientation;
            this->Buttons = ConcurrentData->Buttons;
            this->Pressed = ConcurrentData->Pressed;
            this->Released = ConcurrentData->Released;
            this->TriggerValue = ConcurrentData->TriggerValue;
            this->IsConnected = ConcurrentData->IsConnected;
            this->IsTracking = ConcurrentData->IsTracking;
            this->IsEnabled = ConcurrentData->IsEnabled;
            this->IsCalibrated = ConcurrentData->IsCalibrated;

            this->CycleColourRequest = ConcurrentData->CycleColourRequest;
            this->ResetPoseRequest = ConcurrentData->ResetPoseRequest;
        }
    }

    void ComponentPost()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawControllerData_TLS::ComponentPostAndRead"), 500);
        {
            FScopeLock scopeLock(&ConcurrentData->Lock);

            // Post the component thread's data to the worker thread's data
            ConcurrentData->RumbleRequest = this->RumbleRequest;
            ConcurrentData->ResetPoseRequest = this->ResetPoseRequest;
            ConcurrentData->CycleColourRequest = this->CycleColourRequest;
        }
    }

    void WorkerRead()
    {
        FScopeLock scopeLock(&ConcurrentData->Lock);

        // Read the component thread's data into the worker thread's data
        this->RumbleRequest = ConcurrentData->RumbleRequest;
        this->ResetPoseRequest = ConcurrentData->ResetPoseRequest;
        this->CycleColourRequest = ConcurrentData->CycleColourRequest;
    }

    void WorkerPost()
    {
        FScopeLock scopeLock(&ConcurrentData->Lock);

        // Post the worker thread's data to the component thread's data
        ConcurrentData->PSMovePosition = this->PSMovePosition;
        ConcurrentData->PSMoveOrientation = this->PSMoveOrientation;
        ConcurrentData->Buttons = this->Buttons;
        ConcurrentData->Pressed = this->Pressed;
        ConcurrentData->Released = this->Released;
        ConcurrentData->TriggerValue = this->TriggerValue;
        ConcurrentData->IsConnected = this->IsConnected;
        ConcurrentData->IsCalibrated = this->IsCalibrated;
        ConcurrentData->IsTracking = this->IsTracking;
        ConcurrentData->IsEnabled = this->IsEnabled;

        // Switches to indicate whether or not the pose needs to be reset or the colour needs to be cycled.
        ConcurrentData->ResetPoseRequest = this->ResetPoseRequest;
        ConcurrentData->CycleColourRequest = this->CycleColourRequest;
    }
};

USTRUCT()
struct FPSMoveDataContext
{
    GENERATED_USTRUCT_BODY();
    
    UPROPERTY()
    int32 PSMoveID;
    
    FPSMoveRawSharedData_TLS RawSharedData;
    FPSMoveRawControllerData_TLS RawControllerData;
    
    void ComponentRead()
    {
        if (RawSharedData.IsValid())
        {
            RawSharedData.ComponentRead();
        }
        
        if (RawControllerData.IsValid())
        {
            RawControllerData.ComponentRead();
        }
    }

    void ComponentPost()
    {
        if (RawSharedData.IsValid())
        {
            RawSharedData.ComponentPost();
        }

        if (RawControllerData.IsValid())
        {
            RawControllerData.ComponentPost();
        }
    }

    // Controller Data Functions
    FVector GetPosition()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.PSMovePosition;
        } else {
            return FVector(0.0);
        }
    }

    FQuat GetOrientation()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.PSMoveOrientation;
        }
        else {
            return FQuat::Identity;
        }
    }
    
    FRotator GetRotation()
    {
        return GetOrientation().Rotator();
    }

    bool GetIsTracking()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.IsTracking;
        }
        else {
            return false;
        }
    }
    
    bool GetIsEnabled()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.IsEnabled;
        }
        else {
            return false;
        }
    }
    
    bool GetButtonTriangle()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_TRIANGLE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonCircle()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_CIRCLE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonCross()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_CROSS) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonSquare()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_SQUARE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonSelect()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_SELECT) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonStart()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_START) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonPS()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_PS) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonMove()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_MOVE) != 0;
        } else {
            return 0;
        }
    }
    
    uint8 GetTriggerValue()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.TriggerValue;
        } else {
            return 0;
        }
    }
    
    void SetRumbleRequest(uint8 RequestedRumbleValue)
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            RawControllerData.RumbleRequest = RequestedRumbleValue;
        }
    }

    void SetCycleColourRequest(bool AskForColourCycle)
    {
        if (RawControllerData.IsValid() &&
            RawControllerData.IsConnected)
        {
            RawControllerData.CycleColourRequest = AskForColourCycle;
        }
        else
        {
            RawControllerData.CycleColourRequest = false;
        }
    }

    void SetResetPoseRequest(bool AskForPoseReset)
    {
        if (RawControllerData.IsValid() &&
            RawControllerData.IsConnected)
        {
            RawControllerData.ResetPoseRequest = AskForPoseReset;
        }
        else {
            RawControllerData.ResetPoseRequest = false;
        }
    }

    FPSMoveDataContext()
    {
        Clear();
    }

    void Clear()
    {
        PSMoveID = -1;
        RawControllerData.ConcurrentData = nullptr;
        RawSharedData.ConcurrentData = nullptr;
    }    
};

UCLASS()
class UPSMoveTypes : public UObject
{
    GENERATED_BODY()
};
