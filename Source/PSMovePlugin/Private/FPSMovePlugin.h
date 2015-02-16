#pragma once
#include "IPSMovePlugin.h"
/**
 * The Unreal template plugins keep the header in PSMovePlugin.cpp
 * I prefer to have my header separate to implementation.
 */
class DataCollector;  // Forward declaration. Fully declared in ...?
class PSMoveDelegate;  // Forward declaration. Fully declared in ...?

class FPSMovePlugin : public IPSMovePlugin
{
public:
    /** IModuleInterface implementation */
    void StartupModule();
    void ShutdownModule();

    /** Delegate method to subscribe to event calls, only supports one listener */
    void SetDelegate(PSMoveDelegate* newDelegate);
    void RemoveDelegate();

    /** Manual looping, currently called in main thread. */
    void PSMoveTick(float DeltaTime);

    /** Optional public API for direct module bind */
    //PSMoveData* LatestData(int deviceId);

private:
    DataCollector *collector;
    PSMoveDelegate* psmoveDelegate;

    void DelegateTick(float DeltaTime);
    void DelegateUpdateAllData();
};