#pragma once
#include "IPSMovePlugin.h"

/**
 * The Unreal template plugins keep the header in PSMovePlugin.cpp
 * I prefer to have my header separate to implementation.
 */

class FPSMovePlugin : public IPSMovePlugin
{
public:
    /** IModuleInterface implementation */
    void StartupModule(); // Automatically called on module load.
    void ShutdownModule(); // Automatically called on module unload.

private:

};