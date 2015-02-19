#pragma once
#include "IPSMove.h"

/**
 * The Unreal template plugins keep the header in FPSMove.cpp
 * I prefer to have my header separate to definitions.
 */

class FPSMove : public IPSMove
{
public:
    /** IModuleInterface implementation */
    void StartupModule(); // Automatically called on module load.
    void ShutdownModule(); // Automatically called on module unload.

    /** IPSMove implementation */
    void MoveSetup();  // Activate the device. Turn on the tracker. Setup fusion.
    void MoveUnset();  // Deactivate the device. Turn off the tracker. Unset fusion.
    void MoveTick(float DeltaTime);  // (V1) Polls device. Calculates position and orientation. Fires events.
private:

};