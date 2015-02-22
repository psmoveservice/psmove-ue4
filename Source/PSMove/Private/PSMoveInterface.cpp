#include "PSMovePrivatePCH.h"
#include "IPSMove.h"
#include "PSMoveInterface.h"

UPSMoveInterface::UPSMoveInterface(const class FObjectInitializer& PCIP)
    : Super(PCIP)
{
    
}

/*
void IPSMoveInterface::RefreshPQ()
{
    if (IPSMove::IsAvailable())
    {
        // Can I actually return any data using this mechanism?
        IPSMove::Get().CopyPQ(Position, Orientation);
    }
        //PositionAndOrientationUpdated(Position, Orientation);//This crashes UE4.
}
*/