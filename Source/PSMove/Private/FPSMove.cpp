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

    m_move_count = psmove_count_connected();
    m_moves = (PSMove**)calloc(m_move_count, sizeof(PSMove*));

    m_tracker = psmove_tracker_new();
    m_fusion = psmove_fusion_new(m_tracker, 1., 1000.);
    psmove_tracker_set_exposure(m_tracker, Exposure_LOW);

    for (int i = 0; i<m_move_count; i++)
    {
        m_moves[i] = psmove_connect_by_id(i);
        psmove_enable_orientation(m_moves[i], PSMove_True);
        assert(psmove_has_orientation(m_moves[i]));

        while (psmove_tracker_enable(m_tracker, m_moves[i]) != Tracker_CALIBRATED);
        //TODO: psmove_tracker_enable_with_color(m_tracker, m_moves[i], r, g, b)
        //TODO: psmove_tracker_get_color(m_tracker, m_moves[i],unisgned char &r, &g, &b);
        enum PSMove_Bool auto_update_leds = psmove_tracker_get_auto_update_leds(m_tracker, m_moves[i]);
    }

}

void FPSMove::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // If we had to do any work to load the library in StartupModule, we would undo that work here.
    UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));

    psmove_fusion_free(m_fusion);
    psmove_tracker_free(m_tracker);
    for (int i = 0; i<m_move_count; i++) {
        psmove_disconnect(m_moves[i]);
    }
    free(m_moves);
}

// Activate the device. Turn on the tracker. Setup fusion.
void FPSMove::MoveSetup()
{
    UE_LOG(LogPSMove, Log, TEXT("MoveSetup. TODO: Calibrate."));
}

// Deactivate the device. Turn off the tracker. Unset fusion.
void FPSMove::MoveUnset()
{
    UE_LOG(LogPSMove, Log, TEXT("TODO: Deactivate move, deactivate tracker, deactivate fusion."));

}

// (V1) Polls device. Calculates position and orientation. Fires events.
void FPSMove::MoveTick(float DeltaTime)
{
    float px, py, pr, pd;
    float ow, ox, oy, oz;
    int buttons;
    unsigned int pressed, released;

    // Renew the image on camera
    psmove_tracker_update_image(m_tracker);
    psmove_tracker_update(m_tracker, NULL); // Passing null (instead of m_moves[i]) updates all controllers.

    for (int i = 0; i < m_move_count; i++)
    {
        psmove_tracker_get_position(m_tracker, m_moves[i], &px, &py, &pr);
        pd = psmove_tracker_distance_from_radius(m_tracker, pr);

        psmove_poll(m_moves[i]); // Required to get orientation.
        psmove_get_orientation(m_moves[i], &ow, &ox, &oy, &oz);

        // TODO: Write pos + orientation data to structure and fire event.

        UE_LOG(LogPSMove, Log, TEXT("Controller %d: pos = %f %f px, z=%f cm; orientation wxyz = %f %f %f %f"), px, py, pd, ow, ox, oy, oz);

        buttons = psmove_get_buttons(m_moves[i]);
        psmove_get_button_events(m_moves[i], &pressed, &released);  // i.e., state change

        // e.g., pressed & Btn_CROSS or buttons & Btn_MOVE
        //TODO: Fire button-based events.

     }

}