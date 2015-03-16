#pragma once

#include "PSMoveTypes.generated.h"

//---------------------------------------------------
// Delegate types
//---------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPSMoveDataUpdatedDelegate, int32, MoveID, float, DeltaSeconds);


UCLASS()
class UPSMoveTypes : public UObject
{
	GENERATED_BODY()
};
