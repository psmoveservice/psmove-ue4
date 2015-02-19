#include "PSMovePrivatePCH.h"
#include "IPSMove.h"
#include "PSMoveAbstract.h"

void PSMoveAbstract::MoveSetup()
{
    if (IPSMove::IsAvailable())
    {
        IPSMove::Get().MoveSetup();
    }
}

void PSMoveAbstract::MoveUnset()
{
    if (IPSMove::IsAvailable())
    {
        IPSMove::Get().MoveUnset();
    }
}

void PSMoveAbstract::MoveTick(float DeltaTime)
{
    if (IPSMove::IsAvailable())
    {
        IPSMove::Get().MoveTick(DeltaTime);
    }
}