#pragma once
//#include "PSMoveDelegate.generated.h"

/** Inherit override and set delegate to subscribe to callbacks*/

//Latest Data structure
typedef struct _psmoveData{
    //Raw data

    //Plugin Derived - Values given arm space after arm calibration, otherwise same as raw

    //Body space, useful for easy component space integration
} PSMoveData;

class PSMoveDelegate
{
    friend class FPSMovePlugin;  // Is this necessary?

public:
    /** Events */
    virtual void PSMoveOnConnect(int32 deviceId, uint64 timestamp);
    virtual void PSMoveOnDisconnect(int32 deviceId, uint64 timestamp);
    virtual void PSMoveOnPair(int32 deviceId, uint64 timestamp);
    virtual void PSMoveOnUnpair(int32 deviceId, uint64 timestamp);
    virtual void PSMoveOnData(int32 deviceId, uint64 timestamp, FQuat quat);
    virtual void PSMoveOnData(int32 deviceId, uint64 timestamp, FRotator rot);	//forward rotator version for blueprint events

    /*PSMoveDisabled() will emit if the device was not connected properly upon startup.*/
    virtual void PSMoveDisabled();	

    /** Callable Functions */
/*    virtual PSMoveData* PSMoveLatestData(int32 myoId);
    virtual void PSMoveLatestData(int32 myoId, int32& Pose, FVector& Acceleration, FRotator& Orientation, FVector& Gyro,
                                int32& Arm, int32& xDirection, 
                                FVector& ArmAcceleration, FRotator& ArmOrientation, FVector& ArmGyro, FRotator& ArmCorrection,
                                FVector& BodySpaceAcceleration);
*/

    //Required Functions
    virtual void PSMoveStartup();              // Call this in BeginPlay()
    virtual void PSMoveShutdown();
    virtual void PSMoveTick(float DeltaTime);  // Called every tick.
};