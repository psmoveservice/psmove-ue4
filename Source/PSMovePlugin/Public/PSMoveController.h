#pragma once

#include "PSMoveController.generated.h"

UCLASS(ClassGroup = Input, meta=(BlueprintSpawnableComponent))
class UPSMoveController : public UActorObject
{
    //friend class PSMoveDelegateBlueprint;
    GENERATED_UCLASS_BODY()

public:
    
    ~UPSMoveController();
    virtual void OnRegister() override;
    virtual void OnUnregister() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, meta = (FriendlyName = "setDelegate", CompactNodeTitle = "", Keywords = "set delegate self"), Category = "PSMove Interface")
    void SetInterfaceDelegate(UObject* newDelegate);

private:
    class PSMoveControllerPrivate* _private;
    void InterfaceEventTick(float DeltaTime);

};