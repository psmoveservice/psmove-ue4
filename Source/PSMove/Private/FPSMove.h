#pragma once
#include "PSMovePrivatePCH.h"
#include "IPSMove.h"

/**
 * The Unreal template plugins keep the header in FPSMove.cpp
 * I prefer to have my header separate to definitions.
 */

class FPSMove : public IPSMove
{
public:
    FPSMove();

    /** IModuleInterface implementation */
    void StartupModule(); // Automatically called on module load.
    void ShutdownModule(); // Automatically called on module unload.

    FVector ModulePosition;
    FQuat ModuleOrientation;

    /** IPSMove implementation */
    const FVector GetPosition() const;
    const FQuat GetOrientation() const;
};