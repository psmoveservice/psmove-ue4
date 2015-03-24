#pragma once

#include "ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPSMove, Log, All);

/**
 * The public interface to this module.
 */

class FPSMove : public IModuleInterface //, public IModularFeature?
{
public:
    virtual void StartupModule();
    virtual void ShutdownModule();
    
    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance, loading the module on demand if needed
     */
    static inline FPSMove& Get()
    {
        return FModuleManager::LoadModuleChecked< FPSMove >( "PSMove" );
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
    
    TArray<FVector> ModulePositions;
    TArray<FQuat> ModuleOrientations;
    TArray<uint32> ModuleButtons;
    TArray<uint32> ModulePressed;
    TArray<uint32> ModuleReleased;
    TArray<uint8> ModuleTriggers;
    TArray<uint8> ModuleRumbleRequests;
    
    /**
     * Here we declare functions that will be accessed via the module instance from within the game.
     */
    virtual void InitWorker();
    virtual const FVector GetPosition(uint8 PSMoveID) const;
    virtual const FQuat GetOrientation(uint8 PSMoveID) const;
    virtual const uint32 GetButtons(uint8 PSMoveID) const;
    virtual const uint32 GetPressed(uint8 PSMoveID) const;
    virtual const uint32 GetReleased(uint8 PSMoveID) const;
    virtual const uint8 GetTrigger(uint8 PSMoveID) const;
    virtual void SetRumble(uint8 PSMoveID, uint8 RumbleValue);
};