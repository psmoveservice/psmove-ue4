#pragma once

#include "PSMoveTypes.generated.h"

//---------------------------------------------------
// Delegate types
//---------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPSMoveDataUpdatedDelegate, FVector, Position, FRotator, Rotation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveTriangleButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveCircleButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveCrossButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveSquareButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveSelectButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveStartButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMovePSButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveMoveButtonDelegate, bool, IsDown, bool, IsNewlyPressed, bool, IsNewlyReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPSMoveTriggerButtonDelegate, uint8, Value, bool, IsNewlyPressed, bool, IsNewlyReleased);

UCLASS()
class UPSMoveTypes : public UObject
{
	GENERATED_BODY()
};
