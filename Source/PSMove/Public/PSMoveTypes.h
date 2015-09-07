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
    uint8 TriggerValue;

    UPROPERTY()
    bool IsConnected;           // Whether or not the controller is connected

    UPROPERTY()
    bool IsTracking;            // Whether or not the tracker saw the controller on the latest frame.
    
    UPROPERTY()
    bool IsEnabled;             // Whether or not the tracker is tracking this specific controller

	UPROPERTY()
	int32 SequenceNumber;       // Increments with every update
    
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
        TriggerValue = 0;
        RumbleRequest = 0;
        ResetPoseRequest = false;
        CycleColourRequest = false;
        IsConnected = false;
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

    void ComponentPost()
    {
        FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawControllerData_TLS::ComponentPostAndRead"), 500);
        {
            FScopeLock scopeLock(&ConcurrentData->Lock);

            // Post the component thread's data to the worker thread's data
            ConcurrentData->RumbleRequest = this->RumbleRequest;
            ConcurrentData->ResetPoseRequest = this->ResetPoseRequest;
            ConcurrentData->CycleColourRequest = this->CycleColourRequest;

            // Clear the edge triggered flags after copying to concurrent state
            this->ResetPoseRequest= false;
            this->CycleColourRequest= false;
        }
    }

	void InputManagerPostAndRead()
	{
		FPSMoveHitchWatchdog watchdog(TEXT("FPSMoveRawControllerData_TLS::ComponentPostAndRead"), 500);
		FScopeLock scopeLock(&ConcurrentData->Lock);

		// Post the component thread's data to the worker thread's data
		ConcurrentData->RumbleRequest = this->RumbleRequest;

		// Read the worker thread's data into the component thread's data'
        this->PSMovePosition = ConcurrentData->PSMovePosition;
        this->PSMoveOrientation = ConcurrentData->PSMoveOrientation;
		this->Buttons = ConcurrentData->Buttons;
		this->TriggerValue = ConcurrentData->TriggerValue;
		this->IsConnected = ConcurrentData->IsConnected;
		this->IsTracking = ConcurrentData->IsTracking;
        this->IsEnabled = ConcurrentData->IsEnabled;
		this->SequenceNumber = ConcurrentData->SequenceNumber;
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

		// Since we just updated the controller data, bump the sequence data
		this->SequenceNumber++;

        // Post the worker thread's data to the component thread's data
        ConcurrentData->PSMovePosition = this->PSMovePosition;
        ConcurrentData->PSMoveOrientation = this->PSMoveOrientation;
        ConcurrentData->Buttons = this->Buttons;
        ConcurrentData->TriggerValue = this->TriggerValue;
        ConcurrentData->IsConnected = this->IsConnected;
        ConcurrentData->IsTracking = this->IsTracking;
        ConcurrentData->IsEnabled = this->IsEnabled;
        ConcurrentData->SequenceNumber = this->SequenceNumber;
    }
};

struct FPSMovePose
{
    FVector WorldPosition;
    FQuat WorldOrientation;
    FQuat ZeroYaw;
    FVector LastWorldPosition;
    FQuat LastWorldOrientation;

    void Clear()
    {
        WorldPosition= FVector::ZeroVector;
        WorldOrientation= FQuat::Identity;
        ZeroYaw= FQuat::Identity;
        LastWorldPosition= FVector::ZeroVector;
        LastWorldOrientation= FQuat::Identity;
    }

    void ResetYaw()
    {
        ZeroYaw = LastWorldOrientation;
        ZeroYaw.X = 0;
        ZeroYaw.Y = 0;
        ZeroYaw.Normalize();
        ZeroYaw = ZeroYaw.Inverse();
    }
};

struct FPSMoveDataContext
{
    int32 PSMoveID;
    FPSMovePose Pose;
	bool InputManagerHasProcessedEvents;

    // Controller Data bound to data in the worker thread    
    FPSMoveRawControllerData_TLS RawControllerData;

    // Controller Data from previous frame
	uint32 RawControllerPreviousButtons;
    uint8 RawControllerPreviousTriggerValue;

    // Debug
    bool ShowHMDFrustumDebug;
    bool ShowTrackingDebug;

    FPSMoveDataContext()
    {
        Clear();
    }

    void Clear()
    {
        Pose.Clear();
        PSMoveID = -1;
        RawControllerData.ConcurrentData = nullptr;
        RawControllerPreviousButtons = 0;
        RawControllerPreviousTriggerValue = 0;
        InputManagerHasProcessedEvents= true; // Nothing to process yet
        ShowHMDFrustumDebug= false;
        ShowTrackingDebug= false;
    } 
    
    void ComponentPost()
    {
        if (RawControllerData.IsValid())
        {
            RawControllerData.ComponentPost();
        }
    }

	void InputManagerPostAndRead()
	{
		if (RawControllerData.IsValid())
		{
			// Backup controller state from the previous frame that we want to compute deltas on
			// before it gets stomped by InputManagerPostAndRead()
			int32 PreviousSequenceNumber = RawControllerData.SequenceNumber;
            int32 PreviousTriggerValue= RawControllerData.TriggerValue;
			int32 PreviousButtons = RawControllerData.Buttons;

			// If the worker thread updated the controller, the sequence number will change
			RawControllerData.InputManagerPostAndRead();

			// If the worker thread posted new data then the sequence number should changes
			if (RawControllerData.SequenceNumber != PreviousSequenceNumber)
			{
				// Actually update the previous controller state when we get new data
				RawControllerPreviousTriggerValue = PreviousTriggerValue;
				RawControllerPreviousButtons = PreviousButtons;

				// The input manager has new data to process
				InputManagerHasProcessedEvents = false;
			}
		}
	}

	bool GetInputManagerHasProcessedEvents() const
	{
		return InputManagerHasProcessedEvents;
	}

	void MarkInputManagerHasProcessedEvents()
	{
		InputManagerHasProcessedEvents = true;
	}

    void SetRumbleRequest(uint8 RequestedRumbleValue)
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            RawControllerData.RumbleRequest = RequestedRumbleValue;
        }
    }

    void PostCycleColourRequest()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            RawControllerData.CycleColourRequest = true;
            ComponentPost();
        }
    }

    void PostResetPoseRequest()
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            RawControllerData.ResetPoseRequest = true;
            ComponentPost();
        }
    } 

    void SetShowHMDFrustumDebug(bool flag)
    {
        ShowHMDFrustumDebug= flag;
    }

    void SetShowTrackingDebug(bool flag)
    {
        ShowTrackingDebug= flag;
    }

    // Controller Data Functions
    FVector GetPosition() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.PSMovePosition;
        } else {
            return FVector(0.0);
        }
    }

    FQuat GetOrientation() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.PSMoveOrientation;
        }
        else {
            return FQuat::Identity;
        }
    }
    
    FRotator GetRotation() const
    {
        return GetOrientation().Rotator();
    }

    bool GetIsTracking() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.IsTracking;
        }
        else {
            return false;
        }
    }
    
    bool GetIsEnabled() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.IsEnabled;
        }
        else {
            return false;
        }
    }
    
    bool GetButtonTriangle() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_TRIANGLE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonCircle() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_CIRCLE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonCross() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_CROSS) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonSquare() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_SQUARE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonSelect() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_SELECT) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonStart() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_START) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonPS() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_PS) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonMove() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return (RawControllerData.Buttons & PSMove_Button::Btn_MOVE) != 0;
        } else {
            return 0;
        }
    }

	uint8 GetPreviousTriggerValue() const
	{
		return RawControllerPreviousTriggerValue;
	}
    
    uint8 GetTriggerValue() const
    {
        if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            return RawControllerData.TriggerValue;
        } else {
            return 0;
        }
    }

	// Pressed This Frame
	bool GetButtonPressedThisFrame(PSMove_Button ButtonMask) const
	{
		bool PressedThisFrame = false;

		if (RawControllerData.IsValid() && RawControllerData.IsConnected)
		{
			PressedThisFrame =
				(RawControllerPreviousButtons & ButtonMask) == 0 &&
				(RawControllerData.Buttons & ButtonMask) != 0;
		}

		return PressedThisFrame;
	}

    bool GetButtonTrianglePressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_TRIANGLE);
    }
    
    bool GetButtonCirclePressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_CIRCLE);
    }
    
    bool GetButtonCrossPressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_CROSS);
    }
    
    bool GetButtonSquarePressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_SQUARE);
    }
    
    bool GetButtonSelectPressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_SELECT);
    }
    
    bool GetButtonStartPressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_START);
    }
    
    bool GetButtonPSPressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_PS);
    }
    
    bool GetButtonMovePressed() const
    {
		return GetButtonPressedThisFrame(PSMove_Button::Btn_MOVE);
    }

	bool GetButtonTriggerPressed() const
	{
		return GetButtonPressedThisFrame(PSMove_Button::Btn_T);
	}

	// Released This Frame
	bool GetButtonReleasedThisFrame(PSMove_Button ButtonMask) const
	{
		bool ReleasedThisFrame = false;

		if (RawControllerData.IsValid() && RawControllerData.IsConnected)
		{
			ReleasedThisFrame =
				(RawControllerPreviousButtons & ButtonMask) != 0 &&
				(RawControllerData.Buttons & ButtonMask) == 0;
		}

		return ReleasedThisFrame;
	}

    bool GetButtonTriangleReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_TRIANGLE);
    }
    
    bool GetButtonCircleReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_CIRCLE);
    }
    
    bool GetButtonCrossReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_CROSS);
    }
    
    bool GetButtonSquareReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_SQUARE);
    }
    
    bool GetButtonSelectReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_SELECT);
    }
    
    bool GetButtonStartReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_START);
    }
    
    bool GetButtonPSReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_PS);
    }
    
    bool GetButtonMoveReleased() const
    {
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_MOVE);
    }

	bool GetButtonTriggerReleased() const
	{
		return GetButtonReleasedThisFrame(PSMove_Button::Btn_T);
	}  
};

UCLASS()
class UPSMoveTypes : public UObject
{
    GENERATED_BODY()
};
