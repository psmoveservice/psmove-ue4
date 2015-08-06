#pragma once

#include "PSMoveTypes.generated.h"

//---------------------------------------------------
// Delegate types
//---------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveDataUpdatedDelegate, FVector, Position, FRotator, Rotation, bool, IsTracked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveTriangleButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveCircleButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveCrossButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveSquareButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveSelectButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveStartButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMovePSButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveMoveButtonDelegate, bool, IsDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSMoveTriggerButtonDelegate, uint8, Value);

// Only the raw data frame is needed by the worker.
USTRUCT()
struct FPSMoveRawControllerData_Base
{
    GENERATED_USTRUCT_BODY();
    
	// Worker -> Component
    UPROPERTY()
    float PosX;
    
    UPROPERTY()
    float PosY;
    
    UPROPERTY()
    float PosZ;
    
    UPROPERTY()
    float OriW;
    
    UPROPERTY()
    float OriX;
    
    UPROPERTY()
    float OriY;
    
    UPROPERTY()
    float OriZ;
    
    UPROPERTY()
    uint32 Buttons;
    
    UPROPERTY()
    uint32 Pressed;
    
    UPROPERTY()
    uint32 Released;
    
    UPROPERTY()
    uint8 TriggerValue;

	UPROPERTY()
	bool IsConnected;

	UPROPERTY()
	bool IsCalibrated;

	UPROPERTY()
	bool IsTracked;

	bool LedColourWasUpdated;
	bool PoseWasReset;

	// Component -> Worker
	bool UpdateLedRequest;

	UPROPERTY()
    uint8 RumbleRequest;

    UPROPERTY()
    FLinearColor LedColourRequest;

    UPROPERTY()
    bool ResetPoseRequest;
    //TODO: LEDs
    
    // Constructor
	FPSMoveRawControllerData_Base()
	{
		Clear();
	}

	void Clear()
	{
        PosX = 0;
        PosY = 0;
        PosZ = 0;
        OriW = 0;
        OriX = 0;
        OriY = 0;
        OriZ = 0;
        Buttons = 0;
        Pressed = 0;
        Released = 0;
        TriggerValue = 0;
        RumbleRequest = 0;
        LedColourRequest = FColor();
        LedColourWasUpdated = false;
        UpdateLedRequest = false;
        ResetPoseRequest = false;
        PoseWasReset = false;
        IsConnected = false;
		IsCalibrated = false;
        IsTracked = false;
    }
};

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

USTRUCT()
struct FPSMoveRawControllerData_TLS : public FPSMoveRawControllerData_Base
{
	GENERATED_USTRUCT_BODY();

	// This is a pointer to the 
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

	void ComponentPostAndRead()
	{
		FScopeLock scopeLock(&ConcurrentData->Lock);

		// Post the component thread's data to the worker thread's data
		ConcurrentData->RumbleRequest = this->RumbleRequest;
		ConcurrentData->LedColourRequest = this->LedColourRequest;
		ConcurrentData->ResetPoseRequest = this->ResetPoseRequest;
		ConcurrentData->UpdateLedRequest = this->UpdateLedRequest;

		// Read the worker thread's data into the component thread's data
		this->PosX = ConcurrentData->PosX;
		this->PosY = ConcurrentData->PosY;
		this->PosZ = ConcurrentData->PosZ;
		this->OriW = ConcurrentData->OriW;
		this->OriX = ConcurrentData->OriX;
		this->OriY = ConcurrentData->OriY;
		this->OriZ = ConcurrentData->OriZ;
		this->Buttons = ConcurrentData->Buttons;
		this->Pressed = ConcurrentData->Pressed;
		this->Released = ConcurrentData->Released;
		this->TriggerValue = ConcurrentData->TriggerValue;
		this->IsConnected = ConcurrentData->IsConnected;
		this->IsTracked = ConcurrentData->IsTracked;
		this->IsCalibrated = ConcurrentData->IsCalibrated;
		this->LedColourWasUpdated = ConcurrentData->LedColourWasUpdated;
		this->PoseWasReset = ConcurrentData->PoseWasReset;
	}

	void WorkerRead()
	{
		FScopeLock scopeLock(&ConcurrentData->Lock);

		// Read the component thread's data into the worker thread's data
		this->RumbleRequest = ConcurrentData->RumbleRequest;
		this->LedColourRequest = ConcurrentData->LedColourRequest;
		this->ResetPoseRequest = ConcurrentData->ResetPoseRequest;
		this->UpdateLedRequest = ConcurrentData->UpdateLedRequest;
	}

	void WorkerPost()
	{
		FScopeLock scopeLock(&ConcurrentData->Lock);

		// Post the worker thread's data to the component thread's data
		ConcurrentData->PosX = this->PosX;
		ConcurrentData->PosY = this->PosY;
		ConcurrentData->PosZ = this->PosZ;
		ConcurrentData->OriW = this->OriW;
		ConcurrentData->OriX = this->OriX;
		ConcurrentData->OriY = this->OriY;
		ConcurrentData->OriZ = this->OriZ;
		ConcurrentData->Buttons = this->Buttons;
		ConcurrentData->Pressed = this->Pressed;
		ConcurrentData->Released = this->Released;
		ConcurrentData->TriggerValue = this->TriggerValue;
		ConcurrentData->IsConnected = this->IsConnected;
		ConcurrentData->IsCalibrated = this->IsCalibrated;
		ConcurrentData->IsTracked = this->IsTracked;
		ConcurrentData->LedColourWasUpdated = this->LedColourWasUpdated;
		ConcurrentData->PoseWasReset = this->PoseWasReset;
	}
};

USTRUCT()
struct FPSMoveDataContext
{
    GENERATED_USTRUCT_BODY();
    
    UPROPERTY()
    int32 PSMoveID;
    
	FPSMoveRawControllerData_TLS RawControllerData;
    
	void ComponentPostAndRead()
	{
		if (RawControllerData.IsValid())
		{
			RawControllerData.ComponentPostAndRead();
		}
	}

	// Controller Data Functions
    FVector GetPosition()
    {
		if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
			return FVector(RawControllerData.PosX, RawControllerData.PosY, RawControllerData.PosZ);
        } else {
            return FVector(0.0);
        }
    }

    FQuat GetOrientation()
    {
		if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
            //return FQuat(-RawDataPtr->OriX, RawDataPtr->OriY, RawDataPtr->OriZ, -RawDataPtr->OriW);
			return FQuat(RawControllerData.OriX, RawControllerData.OriY, RawControllerData.OriZ, RawControllerData.OriW);

        }
        else {
            return FQuat::Identity;
        }
    }
    
    FRotator GetRotation()
    {
        GetOrientation().Rotator();
    }

    bool GetIsTracked()
    {
		if (RawControllerData.IsValid() && RawControllerData.IsConnected)
        {
			return RawControllerData.IsTracked;
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

    void SetLedColourRequest(FLinearColor RequestedColourValue)
    {
		if (RawControllerData.IsValid() &&
			RawControllerData.IsConnected &&
			!RawControllerData.LedColourWasUpdated &&
			!(RawControllerData.LedColourRequest == RequestedColourValue) &&
			RequestedColourValue != FLinearColor::Transparent)
        {
            RawControllerData.LedColourRequest = RequestedColourValue;
            RawControllerData.UpdateLedRequest = true;
        }
        else {
			RawControllerData.UpdateLedRequest = false;
        }
    }

    void SetResetPoseRequest(bool AskForPoseReset)
    {
		if (RawControllerData.IsValid() &&
			RawControllerData.IsConnected &&
			!(RawControllerData.ResetPoseRequest == AskForPoseReset) &&
			!RawControllerData.PoseWasReset)
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
	}    
};

UCLASS()
class UPSMoveTypes : public UObject
{
	GENERATED_BODY()
};
