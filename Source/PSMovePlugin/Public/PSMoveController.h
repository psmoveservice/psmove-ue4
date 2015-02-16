#pragma once

#include "Core.h"
#include "Engine.h"
#include "PSMoveDelegate.h"
#include "PSMoveController.generated.h"

/*
UCLASS(ClassGroup = Input, meta=(BlueprintSpawnableComponent))
class UPSMoveController : public UActorObject
{
    //friend class PSMoveDelegateBlueprint;
    GENERATED_UCLASS_BODY()

public:

    //orientation in rotator format
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PSMove")
    FRotator orientation;

    //Conversion
    void setFromPSMoveData(PSMoveData* data);

private:
    PSMoveDelegate* _psmoveDelegate;

};
*/