#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "IPSMove.h"
#include "FPSMove.h"

IMPLEMENT_MODULE( FPSMove, PSMove )

FPSMove::FPSMove()
: m_moves(NULL), m_tracker(NULL), m_fusion(NULL)
{

}

void FPSMove::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    UE_LOG(LogPSMove, Log, TEXT("Loaded PSMove module."));
}

void FPSMove::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // If we had to do any work to load the library in StartupModule, we would undo that work here.
    UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));
}

// Activate the device. Turn on the tracker. Setup fusion.
void FPSMove::MoveSetup()
{
    UE_LOG(LogPSMove, Log, TEXT("TODO: activate move, activate tracker, activate fusion."));
    m_move_count = psmove_count_connected();
    m_tracker = psmove_tracker_new();
    m_fusion = psmove_fusion_new(m_tracker, 1., 1000.);
    psmove_tracker_set_exposure(m_tracker, Exposure_LOW);

    m_moves = (PSMove**)calloc(m_move_count, sizeof(PSMove*));
    for (int i = 0; i<m_move_count; i++) {
        m_moves[i] = psmove_connect_by_id(i);

        psmove_enable_orientation(m_moves[i], PSMove_True);
        assert(psmove_has_orientation(m_moves[i]));

        while (psmove_tracker_enable(m_tracker, m_moves[i]) != Tracker_CALIBRATED);
    }
}

// Deactivate the device. Turn off the tracker. Unset fusion.
void FPSMove::MoveUnset()
{
    UE_LOG(LogPSMove, Log, TEXT("TODO: Deactivate move, deactivate tracker, deactivate fusion."));
    
    psmove_fusion_free(m_fusion);
    psmove_tracker_free(m_tracker);
    for (int i = 0; i<m_move_count; i++) {
        psmove_disconnect(m_moves[i]);
    }
    free(m_moves);
}

// (V1) Polls device. Calculates position and orientation. Fires events.
void FPSMove::MoveTick(float DeltaTime)
{
    UE_LOG(LogPSMove, Log, TEXT("Polling. TODO: get data, fire events."));
    for (int i = 0; i < m_move_count; i++) {
        while (psmove_poll(m_moves[i])); // Why while? Why not just poll once?
        //psmove_fusion_get_position(m_fusion, m_moves[i], &(pos.x), &(pos.y), &(pos.z));
        //psmove_fusion_get_modelview_matrix(m_fusion, m_moves[i]);
    }
}