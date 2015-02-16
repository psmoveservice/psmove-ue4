#include "PSMovePluginPrivatePCH.h"
#include "PSMoveDelegate.h"
#include "IPSMovePlugin.h"
/* Already in PrivatePCH 
#include "IPSMovePlugin.h"
#include "PSMoveDelegate.h"
*/

//DEFINE_LOG_CATEGORY(PSMovePluginLog);

//Input Mapping EKey definitions

//Empty Implementations
void PSMoveOnConnect(int32 deviceId, uint64 timestamp){}
void PSMoveOnDisconnect(int32 deviceId, uint64 timestamp){}
void PSMoveOnPair(int32 deviceId, uint64 timestamp){}
void PSMoveOnUnpair(int32 deviceId, uint64 timestamp){}
void PSMoveOnData(int32 deviceId, uint64 timestamp, FQuat quat){}
void PSMoveOnData(int32 deviceId, uint64 timestamp, FRotator rot){}

void PSMoveDisabled(){}

/*
PSMoveData* PSMoveDelegate::PSMoveLatestData(int32 myoId)
{
    if (IPSMovePlugin::IsAvailable())
    {
        return IPSMovePlugin::Get().LatestData(myoId);
    }
    return NULL;
}
*/

/*
void PSMoveDelegate::PSMoveLatestData(int32 myoId, int32& Pose, FVector& Acceleration, FRotator& Orientation, FVector& Gyro,
                    int32& Arm, int32& xDirection,
                    FVector& ArmAcceleration, FRotator& ArmOrientation, FVector& ArmGyro, FRotator& ArmCorrection,
                    FVector& BodySpaceAcceleration){

    PSMoveData data = *PSMoveLatestData(myoId);
}
*/


void PSMoveDelegate::PSMoveTick(float DeltaTime)
{
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().PSMoveTick(DeltaTime);
    }
}
void PSMoveDelegate::PSMoveStartup()
{
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().SetDelegate((PSMoveDelegate*)this);
    }
}

void PSMoveDelegate::PSMoveShutdown()
{
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().RemoveDelegate();
    }
}