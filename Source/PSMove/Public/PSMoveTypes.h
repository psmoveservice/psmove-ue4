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
struct FPSMoveRawDataFrame
{
    GENERATED_USTRUCT_BODY();
    
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
    uint8 RumbleRequest;

    UPROPERTY()
    FLinearColor LedColourRequest;

    bool LedColourWasUpdated;
    bool UpdateLedRequest;

    UPROPERTY()
    bool ResetPoseRequest;

    bool PoseWasReset;

    UPROPERTY()
    bool IsConnected;
    
    UPROPERTY()
    bool IsTracked;
    
    //TODO: LEDs
    
    // Constructor
    FPSMoveRawDataFrame()
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
        IsTracked = false;
    }
};

USTRUCT()
struct FPSMoveDataFrame
{
    GENERATED_USTRUCT_BODY();
    
    UPROPERTY()
    int32 PSMoveID;
    
    FPSMoveRawDataFrame* RawDataPtr;
    
    FVector GetPosition()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return FVector(RawDataPtr->PosX, RawDataPtr->PosY, RawDataPtr->PosZ);
        } else {
            return FVector(0.0);
        }
    }

    FQuat GetOrientation()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            //return FQuat(-RawDataPtr->OriX, RawDataPtr->OriY, RawDataPtr->OriZ, -RawDataPtr->OriW);
            return FQuat(RawDataPtr->OriX, RawDataPtr->OriY, RawDataPtr->OriZ, RawDataPtr->OriW);

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
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return RawDataPtr->IsTracked;
        }
        else {
            return false;
        }
    }
    
    bool GetButtonTriangle()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_TRIANGLE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonCircle()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_CIRCLE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonCross()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_CROSS) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonSquare()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_SQUARE) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonSelect()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_SELECT) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonStart()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_START) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonPS()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_PS) != 0;
        } else {
            return 0;
        }
    }
    
    bool GetButtonMove()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return (RawDataPtr->Buttons & PSMove_Button::Btn_MOVE) != 0;
        } else {
            return 0;
        }
    }
    
    uint8 GetTriggerValue()
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            return RawDataPtr->TriggerValue;
        } else {
            return 0;
        }
    }
    
    void SetRumbleRequest(uint8 RequestedRumbleValue)
    {
        if (RawDataPtr && RawDataPtr->IsConnected)
        {
            RawDataPtr->RumbleRequest = RequestedRumbleValue;
        }
    }

    void SetLedColourRequest(FLinearColor RequestedColourValue)
    {
        if (RawDataPtr && RawDataPtr->IsConnected && !RawDataPtr->LedColourWasUpdated && !(RawDataPtr->LedColourRequest == RequestedColourValue))
        {
            RawDataPtr->LedColourRequest = RequestedColourValue;
            RawDataPtr->UpdateLedRequest = true;
        }
        else {
            RawDataPtr->UpdateLedRequest = false;
        }
    }

    void SetResetPoseRequest(bool AskForPoseReset)
    {
        if (RawDataPtr && RawDataPtr->IsConnected && !(RawDataPtr->ResetPoseRequest == AskForPoseReset) && !RawDataPtr->PoseWasReset)
        {
            RawDataPtr->ResetPoseRequest = AskForPoseReset;
        }
        else {
            RawDataPtr->ResetPoseRequest = false;
        }
    }

    FPSMoveDataFrame()
    {
        PSMoveID = -1;
    }
    
    void Destroy()
    {
        RawDataPtr = nullptr;
    }
    
};

UCLASS()
class UPSMoveTypes : public UObject
{
	GENERATED_BODY()
};
