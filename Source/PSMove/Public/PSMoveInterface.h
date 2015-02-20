#pragma once

#include "PSMoveInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPSMoveInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IPSMoveInterface
{
    GENERATED_IINTERFACE_BODY()

public:
    FVector Position;
    FQuat Orientation;
    virtual void RefreshPQ(); // 

        //TODO: BlueprintImplementable function PosQuatUpdated
    UFUNCTION(BlueprintImplementableEvent, meta=(FriendlyName = "PSMove Position and Orientation Updated"))
    void PositionAndOrientationUpdated(FVector Position, FQuat Orientation);
};