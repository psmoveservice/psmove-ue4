#pragma once

#include "IPSMove.h"
#include "PSMoveAbstract.h"
#include "PSMoveComponent.generated.h"

UCLASS(ClassGroup="Input Controller", meta=(BlueprintSpawnableComponent))
class UPSMoveComponent : public UActorComponent, public PSMoveAbstract
{
    GENERATED_UCLASS_BODY()

    virtual void OnRegister() override; // Must override so we can call module setup
    virtual void OnUnregister() override; // Must override so we can call module unset
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override; // Must override to call module tick.
};