#include "PSMovePluginPrivatePCH.h"

#include "FPSMovePlugin.h"  // Includes IPSMovePlugin.h
//#include "PSMoveDelegate.h"
#include "psmove.h"
#include "psmove_tracker.h"
#include "psmove_fusion.h"

IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )

#define LOCTEXT_NAMESPACE "PSMovePlugin"

class DataCollector
{
public:
    DataCollector()
    {

    }

    ~DataCollector()
    {

    }

    void ConvertAllData()
    {

    }
};

void FPSMovePlugin::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    //UE_LOG(PSMovePluginLog, Log, TEXT("Loaded PSMove Plugin"));

    collector = new DataCollector;
}


void FPSMovePlugin::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

/* Public API Implementation */
/*
PSMoveData* FPSMovePlugin::LatestData(int deviceId)
{

}
*/


void FPSMovePlugin::SetDelegate(PSMoveDelegate* newDelegate){
    //UE_LOG(PSMovePluginLog, Log, TEXT("PSMove Delegate Set (should only be called once per begin play)."));
    psmoveDelegate = newDelegate;

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

}

void FPSMovePlugin::PSMoveTick(float DeltaTime)
{
    // Get the freshest data.

    // Convert and pass the data to the delegate
    collector->ConvertAllData();

    // Call the delegate
    if (psmoveDelegate != NULL)
    {
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
    }
}

void FPSMovePlugin::DelegateTick(float DeltaTime)
{
    // Update data history

    // Trigger any delegate events

}

#undef LOCTEXT_NAMESPACE