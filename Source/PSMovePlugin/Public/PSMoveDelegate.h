#pragma once
    //#include "PSMoveDelegate.generated.h" // Nothing to generate yet.
DEFINE_LOG_CATEGORY_STATIC(LogPSMovePlugin, Log, All); // Put it here because almost every cpp file imports this.
/** Define events and data structure */

/** Inherit override and set delegate to subscribe to callbacks*/

class PSMoveDelegate
{
    friend class FPSMovePlugin;  // Is this necessary?

public:
    
    /** Event Emitter virtual functions.
     *  Override these functions to receive notifications.
     */

    virtual void PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FVector pos, FQuat quat);

    //Required Functions
    virtual void PSMoveStartup();              // Call this in BeginPlay()
    virtual void PSMoveShutdown();
    virtual void PSMoveTick(float DeltaTime);  // Called every tick.
};