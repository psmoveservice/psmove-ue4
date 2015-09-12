#pragma once

#include "ModuleManager.h"
#include "IMotionController.h"
#include "IInputDeviceModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogPSMove, Log, All);

/**
 * The public interface to this module.
 */

struct FPSMoveDataContext;

class FPSMove : public IInputDeviceModule
{
public:  
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
    
    /**
    * Tell the PSMove module that we want to start listening to this controller.
    *
    * @return True if we can successfully acquire the controller.
    */
    virtual bool AcquirePSMove(int32 PlayerIndex, EControllerHand Hand, FPSMoveDataContext **OutDataContext)
    {
        // implemented in internal class
        return false;
    }
    
    /**
     * Tell the PSMove module that we don't care about listening to this controller anymore.
     */
    virtual void ReleasePSMove(FPSMoveDataContext *DataContext)
    {
    }
};