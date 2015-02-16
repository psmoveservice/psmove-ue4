// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

class PSMoveDelegate;  // Forward declaration.
//struct PSMoveData;  // Forward declaration.

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IPSMovePlugin : public IModuleInterface
{

public:

    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance, loading the module on demand if needed
     */
    static inline IPSMovePlugin& Get()
    {
        return FModuleManager::LoadModuleChecked< IPSMovePlugin >( "PSMovePlugin" );
    }

    /**
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @return True if the module is loaded and ready to use
     */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "PSMovePlugin" );
    }

    /**
     * Public API, implemented in ../Private/FPSMovePlugin.cpp
     * Preferred way of getting data is to subscribe to PSMoveDelegate class through inheritance.
     */
    virtual void SetDelegate(PSMoveDelegate* newDelegate) {};
    virtual void RemoveDelegate(){};
    virtual void PSMoveTick(float DeltaTime) {};
    //virtual PSMoveData* LatestData(int deviceId) { return NULL; };
};
