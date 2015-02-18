#pragma once
#include "IPSMovePlugin.h"

/**
 * The Unreal template plugins keep the header in PSMovePlugin.cpp
 * I prefer to have my header separate to implementation.
 */

class PSMoveDelegate;  // Forward declaration.

class FPSMovePlugin : public IPSMovePlugin
{
public:
    /** IModuleInterface implementation */
    void StartupModule(); // Automatically called on module load.
    void ShutdownModule(); // Automatically called on module unload.

    
    /** Delegate method to subscribe to event calls */
    void SetDelegate(PSMoveDelegate* newDelegate);
    void RemoveDelegate();
    /** Manual looping. Call this in class Tick. */
     void PSMoveTick(float DeltaTime);

private:

    PSMoveDelegate* psmoveDelegate;

    void DelegateTick(float DeltaTime);
    void DelegateUpdateAllData();
};