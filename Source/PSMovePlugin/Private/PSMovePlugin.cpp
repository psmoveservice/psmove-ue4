#include "PSMovePluginPrivatePCH.h"  // Also includes psmove api lib headers.

#include "IPSMovePlugin.h"
#include "FPSMovePlugin.h"

IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )

void FPSMovePlugin::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    UE_LOG(LogPSMovePlugin, Log, TEXT("Loaded PSMove plugin"));

    psmoveDelegate = NULL;

    // Do we need to do anything to load the library before actually calling it?
    // We do not want to connect yet.

}

void FPSMovePlugin::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // If we had to do any work to load the library in StartupModule, we would undo that work here.
    // UE_LOG(LogPSMovePlugin, Log, TEXT("Shutdown PSMove plugin."));
}

/** Public API Implementation **/

void FPSMovePlugin::SetDelegate(PSMoveDelegate* newDelegate){
    psmoveDelegate = newDelegate;
    UE_LOG(LogPSMovePlugin, Log, TEXT("PSMove delegate set. (should only appear once)"));

    // Connect and configure the device.
    //See https://github.com/cboulay/psmoveapi/blob/master/examples/c/test_opengl3.cpp
    //PSMove **moves = psmove_connect_by_id(int id);
    //PSMoveTracker *m_tracker;
    //m_tracker(psmove_tracker_new())
    //PSMoveFusion *m_fusion;
    //m_fusion(psmove_fusion_new(m_tracker, 1., 1000.))
    //psmove_tracker_set_mirror(m_tracker, PSMove_False);
    //psmove_tracker_set_exposure(m_tracker, Exposure_LOW);
    //m_moves = (PSMove**)calloc(m_move_count, sizeof(PSMove*));
    //psmove_enable_orientation()
    //psmove_has_orientation()
    //while (psmove_tracker_enable(m_tracker, m_moves[i]) != Tracker_CALIBRATED);
}

void FPSMovePlugin::RemoveDelegate()
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("PSMove delegate removed."));
}

void FPSMovePlugin::PSMoveTick(float DeltaTime)
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("FPSMovePlugin::PSMoveTick"));
    // Get the data

    /*
     Vector3D pos;
     DelegateTick(DeltaTime);
     if (psmove_poll(m_moves[i]))
     {
     psmove_fusion_get_position(m_fusion, m_moves[i],
     &(pos.x), &(pos.y), &(pos.z));
     int buttons = psmove_get_buttons(m_moves[i]);
     }
     psmove_tracker_update(m_tracker, NULL);
     */
    
    // Reformat and pass the data to the delegate.

    // Call the delegate
    if (psmoveDelegate != NULL)
    {
        DelegateTick(DeltaTime);
    }
}

void FPSMovePlugin::DelegateTick(float DeltaTime)
{

    UE_LOG(LogPSMovePlugin, Log, TEXT("FPSMovePlugin::DelegateTick"));
    // Trigger any delegate events


}