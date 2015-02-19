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

private:

};