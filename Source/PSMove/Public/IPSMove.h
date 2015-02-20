// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPSMove, Log, All);

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IPSMove : public IModuleInterface
{

public:
    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance, loading the module on demand if needed
     */
    static inline IPSMove& Get()
    {
        return FModuleManager::LoadModuleChecked< IPSMove >( "PSMove" );
    }

    /**
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @return True if the module is loaded and ready to use
     */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "PSMove" );
    }

    /**
     * Here we declare functions that will be used to access the module instance from within the game.
     * Implemented in Private/FPSMove.cpp
     */
    virtual void CopyPQ(FVector& InPosition, FQuat& InOrientation) const{};
};
