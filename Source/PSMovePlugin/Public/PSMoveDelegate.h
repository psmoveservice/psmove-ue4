#pragma once
    //#include "PSMoveDelegate.generated.h" // Nothing to generate yet.

/** Define events and data structure */

/** Inherit override and set delegate to subscribe to callbacks*/

class PSMoveDelegate
{
    friend class FPSMovePlugin;  // Is this necessary?

public:
    
    /** Event Emitter virtual functions.
     *  Override these functions to receive notifications.
     */

        //virtual void PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FQuat quat);
        //virtual void PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FRotator rot);	//forward rotator version for blueprint events

    //Required Functions
    virtual void PSMoveStartup();              // Call this in BeginPlay()
    virtual void PSMoveShutdown();
    virtual void PSMoveTick(float DeltaTime);  // Called every tick.
};